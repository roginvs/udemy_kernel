#include <stdint-gcc.h>

uint8_t * const video_addr = (void*)0xB8000;
uint32_t video_index = 0;

uint8_t hex_code(uint8_t c){
    c = c + 0x30;
    if(c > 0x39)
        c = c + 0x07;
    return c;
}

void print_char(uint8_t c){
    video_addr[video_index] = c;
    video_index += 2;
}
void print_hex_byte(uint8_t c) {
    video_addr[video_index] = hex_code((c >> 4) & 0x0F);
    video_addr[video_index + 2] = hex_code(c & 0x0F);
    video_index += 4;    
}

void print_hex_dword(uint32_t d) {
    print_hex_byte( (d >> 24) & 0xFF);
    print_hex_byte( (d >> 16) & 0xFF);
    print_hex_byte( (d >> 8) & 0xFF);
    print_hex_byte( (d >> 0) & 0xFF);
}


short * pcolumns = ((uint16_t *)(0x044A));

void go_to_next_line(){
    video_index = video_index + ((*pcolumns * 2) - (video_index % (*pcolumns * 2)));
}

void print_memory_dump(uint32_t base, uint32_t size){
    video_index = *pcolumns * 2; // Set to second line
    print_char('D');
    print_char('u');
    print_char('m');
    print_char('p');
    print_char(' ');
    
    print_char('0');
    print_char('x');
    
    print_hex_dword(base);
    print_char(' ');
    print_char('0');
    print_char('x');
    print_hex_dword(size);

    uint32_t i;
    for(i = 0; i < size; i++){
        if (i % 16 == 0){
            go_to_next_line();

            print_hex_dword(base + i);
            print_char(':');
            print_char(' ');
        }
        print_hex_byte(*((char*)(base + i)));
        if (i % 4 == 3) {
            print_char(' ');
        }
    }
}