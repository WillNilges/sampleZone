/*
* Hghway to the sample zone.
* Work in progress
* @sgreene570
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <fcntl.h>
#include <ncurses.h>
#include <unistd.h>
#include "playback.h"

#define WINDOW_HEIGHT 20
#define WINDOW_WIDTH 30
#define OFFSET 1
#define SAMPLE_MARKERS "0123456789"

double wavLength(u_int32_t wavSize, u_int32_t byteRate) {
    return (double) wavSize / byteRate;
}

void *playFile(void *fd) {
    int file = *((int *) fd);
    // Read wav file header (44 bytes long)
    wavHeader *header =  calloc(1, 44);
    int out = read(file, header, 44);
    if (out != 44) {
        printf("Error reading wav header\n");
        return NULL;
    }
    double length = wavLength(header->subChunk2Size, header->byteRate);
    //Debug wav header
    printf("%u %u %f\n", header->subChunk2Size, header->byteRate, length); playback(header->sampleRate, header->numChannels, length, file);
    pthread_exit(NULL);
}


static WINDOW *create_newwin(int height, int width, int starty, int startx) {
    WINDOW *local_win = newwin(height, width, starty, startx);
    box(local_win, 0, 0);
    keypad(local_win, TRUE);
    return (local_win);
}

bool checkSymbol(char input, char *symbols, int numSymbols) {
    for (int i = 0; i < numSymbols; i++) {
        if (symbols[i] == input) {
            return true;
        }
    }
    return false;
}

void playAudio(WINDOW *win, char *files[]) {
    for (int y = OFFSET; y < WINDOW_HEIGHT + OFFSET; y++) {
        for (int x = OFFSET; x < WINDOW_WIDTH + OFFSET; x++) {

            usleep(83333);
            wmove(win, y, x);
            wrefresh(win);
            int ch = mvwinch(win, y, x) & A_CHARTEXT;
            pthread_t *thread;
            if (checkSymbol(ch, SAMPLE_MARKERS, sizeof(SAMPLE_MARKERS))) {
                printw("PLAYING\n");
                int fd = open(files[atoi(ch + "")], O_RDONLY);
                pthread_create(thread, NULL, playFile, &fd);
            }
        }
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    int numFiles = argc - 1;
    char *files[numFiles];
    // Parse file name input
    for(int i = 1; i < argc; i++) {
        files[i - 1] = argv[i];
    }

    pthread_t threads[numFiles];
    int fd;

    /*
    for(int i = 0; i < numFiles; i++) {
        fd = open(args[i], O_RDONLY);
        pthread_create(&threads[i], NULL, playFile, &fd);
    }
    pthread_exit(NULL);
    */

    // Ncurses code
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();
    int x = OFFSET;
    int y = OFFSET;
    int ch;
    refresh();
    WINDOW *win = create_newwin(WINDOW_HEIGHT, WINDOW_WIDTH, y, x);
    wmove(win, y, x);
    refresh();
    while((ch = wgetch(win)) != 'q') {
        if (checkSymbol(ch, SAMPLE_MARKERS, sizeof(SAMPLE_MARKERS))) {
            mvaddch(y + 1, x + 1, ch);
        } else if (ch == ' ') {
            playAudio(win, files);
        } else {
            // Vim arrow controls with grid boundaries in mind
            switch(ch) {
                case 'h':
                    if(x - 1 > OFFSET - 1) {
                        x--;
                    }
                    break;
                case 'l':
                    if(x + 1 < OFFSET + WINDOW_WIDTH - 2) {
                        x++;
                    }
                    break;
                case 'j':
                    if(y + 1 < OFFSET + WINDOW_HEIGHT - 2) {
                        y++;
                    }
                    break;
                case 'k':
                    if(y - 1 > OFFSET - 1) {
                        y--;
                    }
                    break;
                }
            wmove(win, y, x);
        }
        // Refresh grid after each cursor move or screen print (important)
        refresh();
    }

    endwin();
    return 0;
}