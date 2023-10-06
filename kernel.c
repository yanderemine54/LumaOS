// Copyright (C) Yanderemine54 2023
// See LICENSE for more information.

#include <stdint.h>
#include <stddef.h>
#include <limine.h>
#include <fonts/font8x8.h>

// Global x and y position to keep track of where to draw in the framebuffer.
size_t fb_x_pos = 0;
size_t fb_y_pos = 0;
 
// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent.
 
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};
 
// GCC and Clang reserve the right to generate calls to the following
// 4 functions even if they are not directly called.
// Implement them as the C specification mandates.
// DO NOT remove or rename these functions, or stuff will eventually break!
// They CAN be moved to a different .c file.
 
void *memcpy(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;
 
    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }
 
    return dest;
}
 
void *memset(void *s, int c, size_t n) {
    uint8_t *p = (uint8_t *)s;
 
    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }
 
    return s;
}
 
void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;
 
    if (src > dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if (src < dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }
 
    return dest;
}
 
int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;
 
    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }
 
    return 0;
}

// strlen implementation. Used for the print function.
size_t strlen(const char* str) {
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}
 
// Halt and catch fire function.
static void hcf(void) {
    asm ("cli");
    for (;;) {
        asm ("hlt");
    }
}

// Puts a pixel on the screen, gets passed a framebuffer, x position, y position and a color.
void putpixel(struct limine_framebuffer* framebuffer, size_t x_pos, size_t y_pos, uint32_t color) {
	uint32_t *fb_ptr = framebuffer->address;
	uint64_t position = x_pos + y_pos*(framebuffer->pitch / 4);
	fb_ptr[position] = color;
}

// Font rendering function. Gets passed the framebuffer handle and an ASCII character and prints it to the screen.
// TODO: - Be able to parse PC Screen Font files and render using that. Requires filesystem driver.
//       - Be able to use every Unicode character, not just ASCII. (draw_character_unicode and depreciate this function?)
void draw_character(struct limine_framebuffer* framebuffer, size_t x_offset, size_t y_offset, char c) {
    // Make that function repeatable.
    if (c == '\n') {
	x_offset = 0;
	y_offset = fb_y_pos + 8;
	fb_x_pos = x_offset;
        fb_y_pos = y_offset;
	return;
    } else {
	x_offset = fb_x_pos;
	y_offset = fb_y_pos;
    }
    // Fetch the character's glyph in font8x8_basic
    char* glyph = font8x8_basic[c];
    // Read bitmap bit-by-bit
    for (size_t y = 0; y < 8; y++) {
	for (size_t x = 0; x < 8; x++) {
            if (glyph[y] & (1 << x)) {
		putpixel(framebuffer, x_offset + x, y_offset + y, 0xffffff);
	    }
	}
    }
    fb_x_pos = x_offset + 8;
    fb_y_pos = y_offset;
}

void print(struct limine_framebuffer* framebuffer, const char* str) {
	for(size_t index = 0; index < strlen(str); index++) {
		draw_character(framebuffer, fb_x_pos, fb_y_pos, str[index]);
	}
}
// The following will be our kernel's entry point.
// If renaming _start() to something else, make sure to change the
// linker script accordingly.
void _start(void) {
    // Ensure we got a framebuffer.
    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }
 
    // Fetch the first framebuffer.
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
 
    // draw_character(framebuffer, fb_x_pos, fb_y_pos, 'E');
    print(framebuffer, "Hello World!\n");
    print(framebuffer, "This print() function works!");
    // We're done, just hang...
    hcf();
}
