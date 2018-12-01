																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																							`define SCREEN_WIDTH 176
`define SCREEN_HEIGHT 144

///////* DON'T CHANGE THIS PART *///////
module DE0_NANO(
	CLOCK_50,
	GPIO_0_D,
	GPIO_1_D,
	KEY
);

//=======================================================
//  PARAMETER declarations
//=======================================================
localparam RED = 8'b111_000_00;
localparam GREEN = 8'b000_111_00;
localparam BLUE = 8'b000_000_11;

//=======================================================
//  PORT declarations
//=======================================================

//////////// CLOCK - DON'T NEED TO CHANGE THIS //////////
input 		          		CLOCK_50;

//////////// GPIO_0, GPIO_0 connect to GPIO Default //////////
output 		    [33:0]		GPIO_0_D;
//////////// GPIO_0, GPIO_1 connect to GPIO Default //////////
input 		    [33:20]		GPIO_1_D;
input 		     [1:0]		KEY;

///// PIXEL DATA /////
reg [7:0]	pixel_data_RGB332 = 1;

///// READ/WRITE ADDRESS /////
reg [14:0] X_ADDR;
reg [14:0] Y_ADDR;
wire [14:0] WRITE_ADDRESS;
reg [14:0] READ_ADDRESS; 

assign WRITE_ADDRESS = X_ADDR + Y_ADDR*(`SCREEN_WIDTH);

///// VGA INPUTS/OUTPUTS /////
wire 			VGA_RESET;
wire [7:0]	VGA_COLOR_IN;
wire [9:0]	VGA_PIXEL_X;
wire [9:0]	VGA_PIXEL_Y;
wire [7:0]	MEM_OUTPUT;
wire			VGA_VSYNC_NEG;
wire			VGA_HSYNC_NEG; //currently not used anywhere else
reg			VGA_READ_MEM_EN;

assign GPIO_0_D[5] = VGA_VSYNC_NEG;
assign VGA_RESET = ~KEY[0];

///// I/O for Img Proc /////
wire [8:0] RESULT;

/* WRITE ENABLE */
reg W_EN = 0;

assign GPIO_0_D[0] = clk_24;

wire camera_href, camera_vsync;
assign camera_href = GPIO_1_D[22];
assign camera_vsync = GPIO_1_D[23];  //do we need to connect this to VGA_VSYNC_NEG
assign camera_pclk = GPIO_1_D[33];

wire [7:0] camera_data;
assign camera_data = /*{GPIO_1_D[24], GPIO_1_D[25],
							 GPIO_1_D[26], GPIO_1_D[27],
							 GPIO_1_D[28], GPIO_1_D[29],
							 GPIO_1_D[30], GPIO_1_D[31]};*/GPIO_1_D[31:24]; 

///////* CREATE ANY LOCAL WIRES YOU NEED FOR YOUR PLL *///////
wire clk_24, clk25, clk50;

///////* INSTANTIATE YOUR PLL HERE *///////
PLL	PLL_inst (
	.inclk0 ( CLOCK_50 ),
	.c0 ( clk_24 ),
	.c1 ( clk_25 ),
	.c2 ( clk_50 )
	);


///////* M9K Module *///////
Dual_Port_RAM_M9K mem(
	.input_data(pixel_data_RGB332),
	.w_addr(WRITE_ADDRESS),
	.r_addr(READ_ADDRESS),
	.w_en(W_EN),
	.clk_W(clk_50),
	.clk_R(clk_25), // DO WE NEED TO READ SLOWER THAN WRITE??
	.output_data(MEM_OUTPUT)
);

///////* VGA Module *///////
VGA_DRIVER driver (
	.RESET(VGA_RESET),
	.CLOCK(clk_25),
	.PIXEL_COLOR_IN(VGA_READ_MEM_EN ? MEM_OUTPUT : RED),//RESULT[2:1] ? (VGA_PIXEL_X < 320 ? (RESULT[0] ? BLUE : RED) : (RESULT[2:1] == 1 ? RED : RESULT[2:1] == 2 ? BLUE : RESULT[2:1] == 3 ? GREEN : 0)) : 0),//RESULT[2:1] == 1 ? RED : RESULT[2:1] == 2 ? BLUE : RESULT[2:1] == 3 ? GREEN : 0),
	.PIXEL_X(VGA_PIXEL_X),
	.PIXEL_Y(VGA_PIXEL_Y),
	.PIXEL_COLOR_OUT({GPIO_0_D[9],GPIO_0_D[11],GPIO_0_D[13],GPIO_0_D[15],GPIO_0_D[17],GPIO_0_D[19],GPIO_0_D[21],GPIO_0_D[23]}),
   .H_SYNC_NEG(GPIO_0_D[7]),
   .V_SYNC_NEG(VGA_VSYNC_NEG)
);

///////* Image Processor *///////
IMAGE_PROCESSOR proc(
	.PIXEL_IN(MEM_OUTPUT),
	.CLK(clk_25),
	.VGA_PIXEL_X(VGA_PIXEL_X),
	.VGA_PIXEL_Y(VGA_PIXEL_Y),
	.VGA_VSYNC_NEG(VGA_VSYNC_NEG),
	.RESULT(RESULT)
);


///////* Update Read Address *///////
always @ (VGA_PIXEL_X, VGA_PIXEL_Y) begin
		READ_ADDRESS = (VGA_PIXEL_X + VGA_PIXEL_Y*`SCREEN_WIDTH);
		if(VGA_PIXEL_X>(`SCREEN_WIDTH-1) || VGA_PIXEL_Y>(`SCREEN_HEIGHT-1))begin
				VGA_READ_MEM_EN = 1'b0;
		end
		else begin
				VGA_READ_MEM_EN = 1'b1;
		end
end

/////* Simple Write *///////
//always @(posedge clk_50) begin
//	W_EN = 1;
//	if (X_ADDR == `SCREEN_WIDTH - 1) begin
//		X_ADDR = 0;
//		if (Y_ADDR == `SCREEN_HEIGHT - 1)
//			Y_ADDR = 0;
//		else
//			Y_ADDR = Y_ADDR + 1;
//	end
//	else
//		X_ADDR = X_ADDR + 1;
//	pixel_data_RGB332 = Y_ADDR < 70 ? RED : GREEN;
//end
//
//
reg        pixel_byte = 0;
reg [15:0] pixel_data_RGB565 = 1;
reg        href_prev = 0;

///////* Downsampler *///////

/*
HREF indicates the start (rising edge) and end (falling edge) of a row of pixels in an image.
VSYNC indicates the start (falling edge) and end (rising edge) of a frame of an image
PCLK decides the speed at which the data output is being transmitted. Its rising edge indicates a new byte is ready to read.
*/

always @(posedge camera_pclk) begin
	//W_EN = 1;
	if (camera_vsync == 1) begin
		pixel_byte = 0;
		X_ADDR = 0;
		Y_ADDR = 0;
		W_EN = 0;
	end
	else if (camera_href == 0 && href_prev == 1) begin
		pixel_byte = 0;
		X_ADDR = 0;
		Y_ADDR = Y_ADDR + 1;
		W_EN = 0;
	end
	else if (camera_href == 1) begin
	   if (pixel_byte == 0) begin
		  pixel_data_RGB565[7:0] = camera_data;
		  pixel_byte = 1;
		  W_EN = 0;
	   end
  	   else  begin
		  pixel_data_RGB565[15:8] = camera_data;
		  
		  X_ADDR = X_ADDR + 1;
		  Y_ADDR = Y_ADDR;
		  pixel_byte = 0;
		  W_EN = 1;
		 
		  pixel_data_RGB332 = { pixel_data_RGB565[15:13],
		                        pixel_data_RGB565[10:8],
								      pixel_data_RGB565[4:3] };
//		  pixel_data_RGB332 <= { pixel_data_RGB565[14:12],
//		                         pixel_data_RGB565[9:7],
//								       pixel_data_RGB565[4:3] };
//		  pixel_data_RGB332 <= { pixel_data_RGB565[15:13],
//		                        pixel_data_RGB565[9:8],
//										 camera_data[7],
//								       camera_data[4:3] };
//			pixel_data_RGB332 <= { pixel_data_RGB565[15:13],
//		                        pixel_data_RGB565[10:8],
//								      camera_data[4:3] };
										
//										pixel_data_RGB332 <= { pixel_data_RGB565[15:13],
//		                        3'b0,2'b0 };
		  //pixel_data_RGB332 <= ((Y_ADDR > 60 && Y_ADDR < 80) || (X_ADDR > 80 && X_ADDR < 100)) ? {pixel_data_RGB565[14:12], 5'd0} : {3'd0, pixel_data_RGB565[9:7], 2'd0};
		  
	   end
   end
		
	href_prev = camera_href;
end



/*
if vsync then
	xaddr = yaddr = pixel_byte = 0;
else if !href then
   pixel_byte = 0;
   xaddr = 0;
	yaddr ++;
else
	pixel_data_565[15:8] = input
	if pixelbyte {
		pixel_data_565[7:0] = input
		pixel_data_rgb332 = {pixel_data_rgb565[15:13], [10:8], [4:3]}
		xaddr ++;
	}
	pixelbyte = !pixelbyte;
*/
	
endmodule 