/*
 * FormData.h
 *
 *  Created on: Oct 29, 2009
 *      Author: Nancy Minderman
 */

#ifndef FORMDATA_H_
#define FORMDATA_H_

#include <basictypes.h>
#include <ucos.h>

#define FORM_NO_DATA 	0xFF
#define FORM_ERROR 		0xFE
#define FORM_OK			0

enum dir { 	UNSET = 0,
		CW = 1,
		CCW = 2,
		STOP = 3
};


class FormData {
public:
	FormData();
	virtual ~FormData();
	BYTE SetMaxRPM(char * rpm);
	int  GetMaxRPM(void);
	BYTE SetMinRPM(char * rpm);
	int  GetMinRPM(void);
	BYTE SetSteps(char * st);
	int  GetSteps (void);
	BYTE SetRotations(char * rot);
	int  GetRotations(void);
	BYTE SetDirection(char * dir);
	BYTE GetDirection(void);
	BYTE GetMode(void);
	BYTE Init(BYTE motor_mode);
	char  isMaxValid(void);
	char  isMinValid(void);
	char  isModeValid(void);
	char  isDirValid(void);
	char  isRotationValid(void);
	char  isStepsValid(void);
	char  isValid(void);
	char hasOnlyNumbers(char *string);
private:
	int  int_maxrpm;
	int  int_minrpm;
	int  int_steps;
	int	 int_rotations;
	BYTE direction;
	BYTE mode;
	char direction_valid;
	char maxRPM_valid;
	char minRPM_valid;
	char rotations_valid;
	char motor_mode_valid;
	char steps_valid;
};

#endif /* FORMDATA_H_ */
