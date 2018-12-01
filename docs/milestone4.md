---
title: Milestone 4
---

Milestone 4: Treasure Detection
===============================

## Goal

The goal of this milestone was to detect whether there is a treasure, and if so, the color and shape of the treasure. 

## Procedure

Treasure presence and color were completed in Lab 4. To determine the shape of a treasure, we examined different sections of the screen. Previously, we had divided the screen into top, middle, and bottom sections of equal size. In determining a treasure’s shape, we first determined the color of that treasure. We then compared the different screen sections using the number of pixels of that color. 

Using a red treasure as example, if the top section had less red pixels than the bottom section, the treasure was determined to be a triangle. If the middle section had more red pixels than the top and bottom sections, the treasure was determined to be a diamond. Finally, if a treasure was detected that was not a triangle or diamond, it was determined to be a square. Pixel buffers were also added to prevent incorrect shape detection. This can be seen in our following implementation:

    if (blue_count[0] + 250 < blue_count[1] && blue_count[1] + 250 < blue_count[2])
        treasure_shape = 2; // TRIANGLE
    else if (blue_count[0] + 250 < blue_count[1] && blue_count[2] + 250 < blue_count[1])
        treasure_shape = 3; // DIAMOND
    else
        treasure_shape = 1; // SQUARE


Shape Detection:

<div class="video"><iframe width="560" height="315" src="https://www.youtube.com/embed/KeySimZ4uKs" frameborder="0" allow="accelerometer; autoplay; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe></div>
