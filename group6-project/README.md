# Group Members
Aditya Kumar Bej, Shivani Kalamadi, Farhad Alemi, Rafael A Diaz Cruz

# Group 6 Firmware
You can find the firmware under "firmware" folder.

# Group 6 Cartridge
We have developed the "snake" game. You can find the catridge under "cartridge" folder. Upload the firmware and upload the cartridge

# Setup Instructions
Run the "make" command in both the firmware and cartridge folder to generate the .strip file for each.
Run the riscv-sim by the ./runsim.sh script. Upload the firmware code and cartridge code

# Gameplay instructionsdfdfd
You can move the snake with the w, a, x, d keys. CMD button pauses the game.
When you hit the walls or accidentally eat your own body, you die. Hit the RST button to reset the game.
The speed of the snake increases every time you eat the food.

# Milestone 3 requirements
You can find the baseline cartridge code under the folder "baseline_cartridge". This will give other developers
an idea on how to develop their games on our OS. There has been few implementations of some apis in the baseline_catridge folder

# Milestone 4 requirements
You can find the port of group 4 and group 7 under ported_cartridge_group4 and ported_cartridge_group7. The folder contains their firmware code and our implementation of the cartridge game (Snake) using their APIs. Please run 'make' in each of the folders to generate the .strip for the cartridge. We have successfully implemented our game completely in group 4 and group 7. Enjoy!

Note - If you encounter a bug that you are not able to move the snake in "group 7" firmware, please hit RST button and start the game! :)

**********IMPORTANT NOTE**********
Please note - This code has been tested on the 7a0019e commit version of the risc-v simulator (https://github.com/UCDClassNitta/riscv-console/tree/7a0019e176f57dc0312361aa495856209b902896)