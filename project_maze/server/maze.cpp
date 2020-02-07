#include <iostream>
#include <vector>
#include <cstdlib>
#include <string>
#include <time.h>

#include "serialport.h"
using namespace std;

#define up 1
#define down 2
#define left 3
#define right 4

#define r 21
#define c 29
// create three vectors to save for row number, column number and direction
vector<int> row;
vector<int> column;
vector<int> block_move;
// an array to save the map
int block[100][100];

// This function setup for the first step to create a maze,
// make the maze fill of walls
void set(){
    for(int i = 0; i <= r + 1; i++){
        for(int j = 0; j <= c + 1; j++){
            block[i][j] = 1;
        }
    }
}

// A function that gives a easy way to push 3 numbers into 3 vectors
void push_block(int x, int y, int z){
    row.push_back(y);
    column.push_back(x);
    block_move.push_back(z);
}

// This function is able to push all 4 blocks around the given block
// in to vectors if they are in the maze. Also push the direction of the
// target block with respect to the given block.
void dig_road(int &x, int &y){
    if(x + 1 <= r){
        // for example, x + 1 means the target block is on the right to the
        // original block
        push_block(x + 1, y, right);
    }
    if(y + 1 <= c){
        push_block(x, y + 1, up);
    }
    if(x - 1 >= 1){
        push_block(x - 1, y, left);
    }
    if(y - 1 >= 1){
        push_block(x, y - 1, down);
    }
}


// The main function that is able to generate random maze 
void generate_maze(){
    // first setup the whole map
    set();
    // set the starting point
    int x = 1;
    int y = 1;
    int randnum;
    int m;
    // clear the wall in the starting point
    block[x][y] = 0;
    // starting to push the block around it 
    dig_road(x, y);
    // generate random seed
    srand((unsigned int)time(0));
    while(row.size() > 0){
        // From all the points that have been push to the vector,
        // choose one to start generating
        randnum = rand() % row.size();
        // use randnum as an index to collect all the x,y and direction
        y = row[randnum];
        x = column[randnum];
        m = block_move[randnum];
        // once we collect the data of one point, we check for the block that
        // is one more step further from the original one.
        // For example, we select the block that contains a direction 'left'
        // we check the block that is 2 steps left to the original block.
        switch (m){
            case up:
                y++;
                break;
            case down:
                y--;
                break;
            case right:
                x++;
                break;
            case left:
                x--;
                break;
        }
        // If that block (farther one) is a wall then break both walls to empty road
        // and push blocks around it again
        if(block[x][y] == 1){
            block[x][y] = 0;
            block[column[randnum]][row[randnum]] = 0;
            dig_road(x, y);
        }
        // After proceeding the whole process, delete the point from the vector
        row.erase(row.begin() + randnum);
        column.erase(column.begin() + randnum);
        block_move.erase(block_move.begin() + randnum);
    }

}

// the main function basically do a communication function
int main(){
    string s, temp;
    int mode = 0;
    SerialPort Serial("/dev/ttyACM0");
    temp = "";
    // At the begining, wait for calling to start
    temp = Serial.readline();
    
    
    if(temp[0] == 'N'){
        mode = 1;
    }

    while(mode){
        // generate random maze
        generate_maze();
        cout << "Drawing the map" <<endl;
        // send it to arduino using serialport
        for(int i = 0; i <= r + 1; i++){
            s = "";
            for(int j = 0; j <= c + 1; j++){
                s += to_string(block[i][j]);
                
            }
            Serial.writeline(s);
            
            temp = Serial.readline(2000);
            
            
        }
        cout << "Done" << endl;
        // When the game is finished, arduino should send a message indicate whether
        // it should go into next another round or quit.
        temp = Serial.readline();
        if(temp[0] == 'E'){
            break;
        }
    }
    return 0;
}