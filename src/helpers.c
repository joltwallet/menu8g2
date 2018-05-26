#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include "u8g2.h"
#include "menu8g2.h"
#include "helpers.h"

uint8_t menu8g2_get_center_x(menu8g2_t *menu, const char *text){
    // Computes X position to print text in center of screen
    u8g2_uint_t width = u8g2_GetStrWidth(menu->u8g2, text);
    return (u8g2_GetDisplayWidth(menu->u8g2)-width)/2 ;
}

bool menu8g2_draw_str(menu8g2_t *menu, const uint16_t x, const uint16_t y, const char *str, const uint16_t line_start){
    /* Takes into account newlines. 
     * Returns True if there's more text offscreen*/
    char *buf = calloc(strlen(str)+1, sizeof(char));
    uint16_t item_height = u8g2_GetAscent(menu->u8g2) - u8g2_GetDescent(menu->u8g2) 
            + CONFIG_MENU8G2_BORDER_SIZE;
    uint16_t y_pos = y;
    uint16_t line = 0;
    bool more_text = false;
    for(size_t i=0, j=0; i<strlen(str)+1; i++){
        if( line >= line_start ){
            buf[j] = str[i];
        }
        if( '\n' == str[i] || '\0' == str[i]){
            buf[j] = '\0';
            if( line >= line_start ){
                u8g2_DrawStr(menu->u8g2, x, y_pos, buf);
                y_pos += item_height;
                if(y_pos >= u8g2_GetDisplayHeight(menu->u8g2) + item_height){
                    more_text = true;
                    break;
                }
            }
            j = 0;
            line++;
        }
        else{
            j++;
        }
    }
    free( buf );
    return more_text;
}
 
char *menu8g2_word_wrap(char* buffer, size_t *buf_len, char* string, int line_width) {
	/*
		This function was derived from a snipped submitted by Sean Hubbard
        on 2012-01-22
		https://www.cprogramming.com/snippets/source-code/word-wrap-in-c

        Sets buf_len to the length that the buffer would need to be.
        buffer can be null to just compute buf_len.
	 
		This function takes a string and an output buffer and a desired width. It then copies 
		the string to the buffer, inserting a new line character when a certain line
		length is reached.  If the end of the line is in the middle of a word, it will
		backtrack along the string until white space is found.
	*/

    uint32_t i = 0, j = 0;
    uint32_t last_space = 0;
 
    while(i < strlen( string ) ) {
        // copy string until the end of the line is reached
        for ( uint32_t x = 0; x < line_width; x++, i++, j++ ) {
            // check if end of string reached
            if ( i == strlen( string ) ) {
                goto exit;
            }
            if( buffer ) {
                buffer[ j ] = string[ i ];
            }
            // check for newlines embedded in original input and reset the index
            if ( string[ i ] == '\n' ) {
                x = 0; 
            }
        }
        // check for whitespace
        if ( ' ' == string[ i ] ) {
            if( buffer ) {
                buffer[j] = '\n';
            }
            i++, j++;
        } 
        else {
            // check for nearest whitespace back in string and replace it with
            // a '\n'
            for ( uint32_t k = i; k > 0; k--) {
                if ( ' ' == string[ k ] ) {
                    if ( last_space == k ) {
                        // We've been here before, we have to break up this
                        // long word
                        if( buffer ) {
                            buffer[ j-1 ] = '\n';
                        }
                        i--;
                    }
                    else {
                        j -= (i-k);
                        if( buffer ) {
                            buffer[ j ] = '\n';
                        }
                        j++;
                        last_space = k;
                        i = last_space + 1;
                    }
                    break;
                }
            }
        }
    }

    exit:
        if( buffer ){
            buffer[ j ] = 0;
        }
        if( buf_len ){
            *buf_len = j;
        }
     
        return buffer;
}
