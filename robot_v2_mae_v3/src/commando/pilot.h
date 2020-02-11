/* -*- mode: c; c-basic-offset: 4; indent-tabs-mode: nil -*- */  
/**
 * @file  pilot.h
 *
 * @brief  description
 *
 * @author Jerome Delatour
 * @date 17-04-2016
 * @version version
 * @section License
 *
 * The MIT License
 *
 * Copyright (c) 2016, Jerome Delatour
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 */

#ifndef PILOT_H
#define PILOT_H

typedef enum {LEFT=0, RIGHT, FORWARD, BACKWARD, STOP, NB_DIR} Direction;
    
typedef struct
{
    Direction dir;
    int power;
} VelocityVector;

typedef struct
{
    int speed;
    int collision;
    float luminosity;
} PilotState;

/**
 * Start Pilot
 *
 */
extern void Pilot_start();


/**
 * Stop Pilot
 *
 */
extern void Pilot_stop();


/**
 * initialize in memory the object Pilot
 */
extern void Pilot_new();


/**
 * destruct the object Pilot from memory 
 */
extern void Pilot_free();

/**
 * setVelocity
 * 
 * @brief description 
 * @param vel 
 */
extern void Pilot_setVelocity(VelocityVector vel);

/**
 * getState
 * 
 * @brief description 
 * @return PilotState
 */
extern PilotState Pilot_getState();

/**
 * check
 * 
 * @brief description 
 */
extern void Pilot_check();



#endif /* PILOT_H */

