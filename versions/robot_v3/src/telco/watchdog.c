/**
 * @file compute_pi.c
 *
 * @author team FORMATO, ESEO
 *
 * @section License
 *
 * The MIT License
 *
 * Copyright (c) 2016, Jonathan ILIAS-PILLET (ESEO)
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
 */
#include "watchdog.h"
#include "common_data.h"
#include <malloc.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

struct Watchdog_t
{
	timer_t timerid;				// Id du watchdog
	uint32_t myDelay; 				// Delais d'attente avant lancement du callback
	WatchdogCallback myCallback;	// Fonction appellee a expiration du delais
};

char err_msg[ERROR_MSG_MAX_LENGTH];

/**
 * Calls the watchdog callback when the delay of the timer expires
 *
 * @param handlerParam must be the watchdog reference
 */
static void mainHandler (union sigval handlerParam)
{
	Watchdog *theWatchdog = handlerParam.sival_ptr;	// On fait référence au timer

	trace("---Watchdog triggered---\n");	

	theWatchdog->myCallback (theWatchdog);
}

Watchdog *Watchdog_construct (uint32_t thisDelay, WatchdogCallback callback)
{
	Watchdog *result;
	int error_code;

	// allocates and initializes the watchdog's attributes
	result = (Watchdog *)malloc (sizeof(Watchdog));
	/* handle error */
    if(errno != 0){
        strcpy(err_msg, "Erreur d'allocation mémoire pour le watchdog...");
        handle_error(errno, err_msg);
    }

	result->myDelay = thisDelay;
	result->myCallback = callback;

	struct sigevent sev = {
		.sigev_notify = SIGEV_THREAD,			// Notification : un thread
		.sigev_notify_function = mainHandler, 	// Fonction utilisée par le thread
		.sigev_value.sival_ptr = result			// Argument passé à la fonction (référence au watchdog)
	};

	error_code = timer_create(CLOCK_REALTIME, &sev, &result->timerid);	// Création du timer utilisé par le watchdog
	/* handle error */
    if(error_code != 0){
        strcpy(err_msg, "Erreur de création du timer du watchdog...");
        handle_error(errno, err_msg);
    }

	return result;
}

void Watchdog_start (Watchdog *this)
{
	int error_code;

	struct itimerspec its = {
		.it_value.tv_sec = this->myDelay	// On assigne la valeur du délais d'expiration du watchdog
	};

	error_code = timer_settime(this->timerid, 0, &its, 0); // Démarrage du timer
	/* handle error */
    if(error_code != 0){
        strcpy(err_msg, "Erreur de démarrage du timer du watchdog...");
        handle_error(errno, err_msg);
    }

}

void Watchdog_cancel (Watchdog *this)
{
	int error_code;

	struct itimerspec its = {};	// Mise a zero du délais
	error_code = timer_settime(this->timerid, 0, &its, 0); // Arrêt du timer
   	/* handle error */
    if(error_code != 0){
        strcpy(err_msg, "Erreur d'arrêt du timer du watchdog...");
        handle_error(errno, err_msg);
    }

}

void Watchdog_destroy (Watchdog *this)
{
	int error_code;
	error_code = timer_delete(this->timerid);	// Destruction du timer
	/* handle error */
    if(error_code != 0){
        strcpy(err_msg, "Erreur de destruction du watchdog...");
        handle_error(errno, err_msg);
    }

	free (this);
}
