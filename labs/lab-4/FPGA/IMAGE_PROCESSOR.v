`define SCREEN_WIDTH 176
`define SCREEN_HEIGHT 144
`define NUM_BARS 3
`define BAR_HEIGHT 48

module IMAGE_PROCESSOR (
	PIXEL_IN,
	CLK,
	VGA_PIXEL_X,
	VGA_PIXEL_Y,
	VGA_VSYNC_NEG,
	RESULT
);


//=======================================================
//  PORT declarations
//=======================================================
input	[7:0]	PIXEL_IN;
input 		CLK;

input [9:0] VGA_PIXEL_X;
input [9:0] VGA_PIXEL_Y;
input			VGA_VSYNC_NEG;

output reg [8:0] RESULT;

wire is_white, is_red, is_blue;
assign is_white = PIXEL_IN[7] && PIXEL_IN[4] && PIXEL_IN[1];
assign is_red = !is_white && PIXEL_IN[7];
assign is_blue = !is_white && (PIXEL_IN[1] | PIXEL_IN[0]);

reg [13:0] red_count [2:0];
reg [13:0] blue_count [2:0];

wire section;
//assign section = VGA_PIXEL_Y < 48 ? 0 : VGA_PIXEL_Y < 96 ? 1 : 2;
//assign section = VGA_PIXEL_Y < 60 ? 0 : VGA_PIXEL_Y > 84 ? 2 : 1;
assign section = VGA_PIXEL_Y < 60 ? 1 : 2;

wire is_treasure, treasure_color;
wire [13:0] diff_mid, abs_diff_mid;

reg [1:0] treasure_shape;

assign diff_mid = red_count[1] - blue_count[1];
assign abs_diff_mid = diff_mid[13] ? -diff_mid : diff_mid;

assign is_treasure = abs_diff_mid > 1000;
assign treasure_color = diff_mid[13]; // RED : 0, BLUE : 1

always @(posedge CLK) begin

	if (VGA_VSYNC_NEG) begin
		if (VGA_PIXEL_X == 0 && VGA_PIXEL_Y == 0) begin
		
			if (1) begin
				if (treasure_color) begin
					if (blue_count[0] > blue_count[2])/// && blue_count[1] + 250 < blue_count[2])
						treasure_shape = 2; // TRIANGLE
					else if (blue_count[0] + 250 < blue_count[1] && blue_count[2] + 250 < blue_count[1])
						treasure_shape = 3; // DIAMOND
					else
						treasure_shape = 1; // SQUARE
				end
				else begin
					if (red_count[0] > red_count[2])/// && red_count[1] + 250 < red_count[2])
						treasure_shape = 2; // TRIANGLE
					else if (red_count[0] + 250 < red_count[1] && red_count[2] + 250 < red_count[1])
						treasure_shape = 3; // DIAMOND
					else
						treasure_shape = 1; // SQUARE
				end
				RESULT[0] = treasure_color;
				RESULT[2:1] = treasure_shape;
			end else
				RESULT = 0;
			
			red_count[0] = 0;
			red_count[1] = 0;
			red_count[2] = 0;
			blue_count[0] = 0;
			blue_count[1] = 0;
			blue_count[2] = 0;
		end
		
		if (VGA_PIXEL_Y > 36 && VGA_PIXEL_Y < 108 && VGA_PIXEL_X < `SCREEN_WIDTH) begin// && VGA_PIXEL_Y < `SCREEN_HEIGHT) begin
			if (is_red)
				red_count[section] = red_count[section] + 1;
			else if (is_blue)
				blue_count[section] = blue_count[section] + 1;
		end
	end
end

//////////////////////////////////////////////////////////////

endmodule