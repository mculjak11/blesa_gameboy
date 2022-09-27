/**
* @file screen_aasi.h
*
* @brief See the source file.
* 
* COPYRIGHT NOTICE: (c) 2022 Byte Lab Grupa d.o.o.
* All rights reserved.
*/

#ifndef __SCREEN_AASI_H__
#define __SCREEN_AASI_H__

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------- INCLUDES ----------------------------------
#include "gui/gui.h"
//---------------------------------- MACROS -----------------------------------

//-------------------------------- DATA TYPES ---------------------------------

//---------------------- PUBLIC FUNCTION PROTOTYPES ---------------------------
/**
 * This function sets the number of aliens in the AASI game
 * 
 * @param num_of_aliens The number of aliens to spawn in the 
 *                      AASI game.
 */
void aasi_game_set_aliens(uint8_t num_of_aliens);

/**
 * This function sets the number of blocks in the AASI game
 * 
 * @param num_of_blocks The number of blocks in the AASI game.
 */
void aasi_game_set_blocks(uint8_t num_of_blocks);

/**
 * Sets the color of the AASI game
 * 
 * @param game_color The color of the AASI game.
 */
void aasi_game_set_game_color(lv_color_t game_color);

/**
 * Sets the color of the object in AASI game
 * 
 * @param object_color The color of the object of the AASI game.
 */
void aasi_game_set_object_color(lv_color_t object_color);


#ifdef __cplusplus
}
#endif

#endif // __SCREEN_AASI_H__
