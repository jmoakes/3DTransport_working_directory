/*
 *  integration.h
 *  flowVC
 *
 *  Created by Shawn Shadden.
 *  Copyright 2010 Flow Physics Group. All rights reserved.
 *
 */

#ifndef INC_INTEGRATION_H
#define INC_INTEGRATION_H

void Advect(LagrangianPoint *pt, double StartTime, double tstart, double tend, int ss, double x1, double x2, double x3);
double pEuler(LagrangianPoint *pt, double tstart, double tend, int ss, double StartTime, double x1, double x2, double x3);
double Euler(LagrangianPoint *pt, double tstart, double tend);
double RK4(LagrangianPoint *pt, double tstart, double tend);
double RKF(LagrangianPoint *pt, double tstart, double tend);

#endif
