// Prange's ISD
double Prange(double k, double w, double MINvalues[32], double *T) {
	if (w==0) {
        w = inverse( 1-k);
		if (HDFlag==1) w /= 2;
    }
	*T = H(w) - H(w/(1-k))*(1-k);
	
	MINvalues[0]=k; MINvalues[1]=w;
}
 
 // BJMM(d) [BJMM12, MO15, BM17]
double BJMMPlus(double k, double w, int d, double spaceBound, double pmin, double pmax, double psteps, double lmin, double lmax, double lsteps, double emin[MaxDepth], double emax[MaxDepth], double esteps[MaxDepth], double MINvalues[32], double MINParams[20], double *T) {
     
    if (w==0) {
        w = inverse( 1-k);
        if (HDFlag) w /= 2;
    }
    double S[MaxDepth+1], C[MaxDepth+1], Ttemp, Stemp;
    double maximum, Smax;
    double l,invprob;
    double p[MaxDepth+1], eps[MaxDepth+1], r[MaxDepth+1], epsswap;
	int check_c;
    *T = 1.0;
 
	for (p[0]=pmin; p[0]<=pmax+psteps*0.01; p[0] = p[0] + psteps) {
	for (l=fmin(lmax,1-k); l>=lmin-lsteps*0.01; l-=lsteps) {
		invprob = H(w) - H(p[0]/(k+l)) * (k+l) - H((w-p[0])/(1-k-l)) * (1-k-l);
		for (eps[1]=emin[1]; eps[1]<=emax[1]+esteps[1]*0.01; eps[1] = eps[1] + esteps[1]) {
		for (eps[2]=emin[2]; eps[2]<=emax[2]+esteps[2]*0.01; eps[2] = eps[2] + esteps[2]) {
		for (eps[3]=emin[3]; eps[3]<=emax[3]+esteps[3]*0.01; eps[3] = eps[3] + esteps[3]) {
			epsswap=eps[d], eps[d] = 0.0;
			r[d] = 0, r[0] = l;
			for (int i=0; i<d; i++) {
				p[i+1] = p[i]/2 + eps[i+1];
			}
			check_c = 0;
			for(int i=1; i<d; i++) {
				r[i] = p[i-1] + H(eps[i]/(k+l-p[i-1])) * (k+l-p[i-1]);
				S[i] = H(p[i]/(k+l)) * (k+l) - r[i];
				C[i] = fmax(0, 2*S[i] + r[i] - r[i-1]);
				if (p[i]/(k+l) > 1 || eps[i]/(k+l-p[i-1]) > 1 || k+l-p[i-1] < 0 || S[i] < 0) {
					check_c = 1;
					break;
				}
				if ((i==1) && (NNFlag==1)) {
					Stemp = S[i]/(1-k-l);
					Ttemp = NN((w-p[i-1])/(1-k-l), S[i]/(1-k-l),  NaiveFlag, &Stemp) * (1-k-l);
					if (C[i] > Ttemp) {
						C[i] = Ttemp;
						S[i] = Stemp*(1-k-l);
					}
				}
			}
			if (check_c) continue;
			
			if (l >= r[1]) {
				S[d] = H(2*p[d]/(k+l))*(k+l)/2;
				C[d] = fmax(0, 2*S[d] + r[d] - r[d-1]);
				if (ErrorFlag==1) {
					ErrorFlag=0;
				}
				maximum = 0.0;
				Smax = 0.0;
				for (int i=1; i<=d; i++) {Smax = fmax(Smax, S[i]);}
				for (int i=1; i<=d; i++) {maximum = fmax(maximum, C[i]);}
				maximum = fmax(maximum,Smax);
				if (invprob >= 0) {
					if (*T > (maximum + invprob) && Smax <= spaceBound) {
						*T = maximum + invprob;
						MINvalues[0]=k; MINvalues[1]=w; MINvalues[2]=l; // for d=3
						for(int i=1; i<d; i++) MINvalues[i+2]=r[i]; // 3,4
						for(int i=0; i<=d; i++) MINvalues[d+2+i]=p[i]; // 5,6,7,8
						for(int i=1; i<=d; i++) MINvalues[2*d+2+i]=S[i]; // 9,10,11
						for(int i=1; i<=d; i++) MINvalues[3*d+2+i]=C[i]; // 12,13,14
						MINvalues[4*d+3]= *T;                             // 15
						MINvalues[4*d+4] = invprob;
						
						MINParams[0] = p[0];
						MINParams[4] = l;
						MINParams[7] = eps[1];
						MINParams[8] = eps[2];
						MINParams[9] = eps[3];
					}
				}// else printf("Invprob<0!!!!\n");
			}
			eps[d] = epsswap;
		}
		}
		}
	}
    }
 
    return 0.0;
 
}
	
// Our Algorithm
double NewV3(double k, double w, int d, double spaceBound, double pmin, double pmax, double psteps, double lmin[MaxDepth], double lmax[MaxDepth], double lsteps[MaxDepth], double emin[MaxDepth], double emax[MaxDepth], double esteps[MaxDepth], double wwmin[MaxDepth][MaxDepth], double wwmax[MaxDepth][MaxDepth], double wwsteps[MaxDepth][MaxDepth], double MINvalues[32], double MINParams[20], double *T) {
    
    if (w==0) {
        w = inverse( 1-k);
		if (HDFlag==1) w /= 2;
    }
    int check_c;
	long int opti_steps=0;
    double S[MaxDepth+1], C[MaxDepth+1], Smax, Stemp;
    double maximum, invprob;
    double l[MaxDepth], p[MaxDepth+1], eps[MaxDepth+1], r[MaxDepth], ww[MaxDepth][MaxDepth], epsswap, lswap, wwsav, wq, lq;
    *T = 1.0;
    for (p[0]=fmax(0,pmin); p[0]<=fmin(fmin(w,k),pmax+psteps*0.01); p[0] = p[0] + psteps) {
		//printf("-- p[0]: %4f \n", p[0]);
	for (ww[1][0]=fmax(0,wwmin[1][0]); ww[1][0]<=fmin(wwmax[1][0],w-p[0])+wwsteps[1][0]*0.01; ww[1][0]+=wwsteps[1][0]) {
	for (ww[2][0]=fmax(0,wwmin[2][0]); ww[2][0]<=fmin(wwmax[2][0],w-p[0]-ww[1][0])+wwsteps[2][0]*0.01; ww[2][0]+=wwsteps[2][0]) {
	for (ww[3][0]=fmax(0,wwmin[3][0]); ww[3][0]<=fmin(wwmax[3][0],w-p[0]-ww[1][0]-ww[2][0])+wwsteps[3][0]*0.01; ww[3][0]+=wwsteps[3][0]) {
	for (ww[4][0]=fmax(0,wwmin[4][0]); ww[4][0]<=fmin(wwmax[4][0],w-p[0]-ww[1][0]-ww[2][0]-ww[3][0])+wwsteps[4][0]*0.01; ww[4][0]+=wwsteps[4][0]) {
		
	for (l[1]=fmin(lmax[1],1-k-(w-p[0]-ww[1][0]-ww[2][0]-ww[3][0]-ww[4][0])); l[1]>=fmax(ww[1][0],lmin[1]-lsteps[1]*0.01); l[1]-=lsteps[1]) {
	for (l[2]=fmin(lmax[2],1-k-(w-p[0]-ww[1][0]-ww[2][0]-ww[3][0]-ww[4][0])-l[1]); l[2]>=fmax(ww[2][0],lmin[2]-lsteps[2]*0.01); l[2]-=lsteps[2]) {
	for (l[3]=fmin(lmax[3],1-k-(w-p[0]-ww[1][0]-ww[2][0]-ww[3][0]-ww[4][0])-l[1]-l[2]); l[3]>=fmax(ww[3][0],lmin[3]-lsteps[3]*0.01); l[3]-=lsteps[3]) {
	for (l[4]=fmin(lmax[4],1-k-(w-p[0]-ww[1][0]-ww[2][0]-ww[3][0]-ww[4][0])-l[1]-l[2]-l[3]); l[4]>=fmax(ww[4][0],lmin[4]-lsteps[4]*0.01); l[4]-=lsteps[4]) {
		lswap=l[d]; l[d] = 0.0;
		wq = w-p[0];
		lq = 1-k;
		for (int i=1; i<d; i++) {
			wq -= ww[i][0];
			lq -= l[i];
		}
		invprob = H(w) - H(p[0]/k) * k - H(wq/lq) * lq;
		for (int i=1; i<d; i++) {
			if (l[i] == 0) continue;
			invprob -= H(ww[i][0]/l[i]) * l[i];
		}

		for (eps[1]=fmax(0,emin[1]); eps[1]<=fmin(k-p[0],emax[1]+esteps[1]*0.01); eps[1] = eps[1] + esteps[1]) {
			p[1] = p[0]/2 + eps[1];
		for (eps[2]=fmax(0,emin[2]); eps[2]<=fmin(k-p[1],emax[2]+esteps[2]*0.01); eps[2] = eps[2] + esteps[2]) {
			p[2] = p[1]/2 + eps[2];
		for (eps[3]=fmax(0,emin[3]); eps[3]<=fmin(k-p[2],emax[3]+esteps[3]*0.01); eps[3] = eps[3] + esteps[3]) {
			p[3] = p[2]/2 + eps[3];
		for (eps[4]=fmax(0,emin[4]); eps[4]<=fmin(k-p[3],emax[4]+esteps[4]*0.01); eps[4] = eps[4] + esteps[4]) {
			p[4] = p[3]/2 + eps[4];
			epsswap=eps[d], eps[d] = 0.0;
			p[d] = p[d-1]/2;
			opti_steps++;
			for (int i=1; i<d; i++) {
				r[i] = p[i-1] + H(eps[i]/(k-p[i-1])) * (k-p[i-1]);
			}

			// only for m=2,3,4,5
			if (d==5) {
				ww[4][3] = l[4]-r[4];
				ww[4][1] = ww[4][2] = ww[4][3];
				ww[4][4] = ww[4][3]/2;
				
				ww[3][2] = l[4]+l[3]-r[3]-ww[4][2]
						 - (l[4]-ww[4][2]) * H((ww[4][3]-ww[4][2]/2) / (l[4]-ww[4][2]));
				ww[3][1] = ww[3][2];
				ww[3][3] = ww[3][2]/2;
				
				ww[2][1] = l[2]+l[3]+l[4]-r[2]-ww[4][1]-ww[3][1]
						 - (l[4]-ww[4][1]) * H((ww[4][2]-ww[4][1]/2) / (l[4]-ww[4][1]))
						 - (l[3]-ww[3][1]) * H((ww[3][2]-ww[3][1]/2) / (l[3]-ww[3][1]));
				ww[2][2] = ww[2][1]/2;
				
				wwsav = (l[1]+l[2]+l[3]+l[4]-ww[4][0]-ww[3][0]-ww[2][0]-ww[1][0]-r[1]
					  - (l[4]-ww[4][0]) * H((ww[4][1]-ww[4][0]/2) / (l[4]-ww[4][0]))
					  - (l[3]-ww[3][0]) * H((ww[3][1]-ww[3][0]/2) / (l[3]-ww[3][0]))
					  - (l[2]-ww[2][0]) * H((ww[2][1]-ww[2][0]/2) / (l[2]-ww[2][0])))
					  / (l[1]-ww[1][0]);
				if (wwsav<0 || wwsav>1) {
					continue;
				}
				ww[1][1] = inverse(wwsav) * (l[1]-ww[1][0]) + ww[1][0]/2;
			} else if (d==4) {
				ww[3][2] = l[3]-r[3];
				ww[3][1] = ww[3][2];
				ww[3][3] = ww[3][2]/2;
				
				ww[2][1] = l[3]+l[2]-r[2]-ww[3][1]
				         - (l[3]-ww[3][1])*H((ww[3][2]-ww[3][1]/2)/(l[3]-ww[3][1]));
				ww[2][2] = ww[2][1]/2;
				
				wwsav = (r[1]-l[1]-l[2]-l[3]+ww[1][0]+ww[2][0]+ww[3][0]
				      + (l[2]-ww[2][0])*H((ww[2][1]-ww[2][0]/2)/(l[2]-ww[2][0]))
					  + (l[3]-ww[3][0])*H((ww[3][1]-ww[3][0]/2)/(l[3]-ww[3][0])))
					  / (ww[1][0]-l[1]);
				if (wwsav<0 || wwsav>1) {
					continue;
				}
				ww[1][1] = inverse(wwsav) * (l[1]-ww[1][0]) + ww[1][0]/2;
			} else if (d==3) {
				ww[2][1] = l[2]-r[2];
				ww[2][2] = ww[2][1]/2;
				wwsav = (r[1]-l[1]-l[2]+ww[1][0]+ww[2][0]
				      + (l[2]-ww[2][0])*H((ww[2][1]-ww[2][0]/2)/(l[2]-ww[2][0])))
					  / (ww[1][0]-l[1]);
				if (wwsav<0 || wwsav>1) {
					continue;
				}
				ww[1][1] = inverse(wwsav) * (l[1]-ww[1][0]) + ww[1][0]/2;
			} else if (d==2) {
				wwsav = (r[1]-l[1]+ww[1][0])/(ww[1][0]-l[1]);
				if (wwsav<0 || wwsav>1) {
					continue;
				}
				ww[1][1] = inverse(wwsav) * (l[1]-ww[1][0]) + ww[1][0]/2;
			}// else printf("Wrong depth! \n");
			
			S[d] = H(2*p[d]/k) * k/2;
			S[d-1] = H(p[d-1]/k) * k + (H(ww[d-1][d-1]/l[d-1]) - 1) * l[d-1];
			if (S[d-1] < 0) continue;
			check_c = 0;
			for (int i=d-2; i>=1; i--) {
				if (ww[i][i]/l[i]<0 || ww[i][i]/l[i]>1) {
					check_c = 1;
					break;
				}
				S[i] = H(p[i]/k) * k + (H(ww[i][i]/l[i]) - 1) * l[i];
				for (int j=i+1; j<d ;j++) {
					if (ww[j][i+1] == 0.0 && ww[j][i] == 0.0) continue;
					if (1-ww[j][i]/ww[j][i+1]/2 < 0 || 1-ww[j][i]/ww[j][i+1]/2 > 1 || ww[j][i]/2/(l[j]-ww[j][i+1]) < 0 || ww[j][i]/2/(l[j]-ww[j][i+1]) > 1 || ww[j][i+1]/(l[j]) < 0 || ww[j][i+1]/(l[j]) > 1) {
						check_c = 1;
						break;
					}
					S[i] += H(1-ww[j][i]/ww[j][i+1]/2) * ww[j][i+1] 
					      + H(ww[j][i]/2/(l[j]-ww[j][i+1])) * (l[j]-ww[j][i+1]) 
						  - H(ww[j][i+1]/l[j]) * l[j];
				}
				if (!check_c && S[i]>0) continue;
				check_c = 1;
				break;
			}
			if (check_c) continue;
			
			maximum = 0.0;
			Smax = 0.0;
			for (int i=2; i<=d; i++) {
				if (l[i-1] == 0) {
					C[i] = 2*S[i];
					Smax = fmax(Smax, S[i]);
				} else {
					Stemp = S[i]/l[i-1];
					C[i] = NN(ww[i-1][i-1]/l[i-1], S[i]/l[i-1],  NaiveFlag, &Stemp) * l[i-1];
					Smax = fmax(Smax, Stemp * l[i-1]);
				}
				maximum = fmax(maximum, C[i]);
			}
			
			Smax = fmax(Smax, S[1]);
			Stemp = S[1]/lq;
			C[1] = NN(wq/lq, S[1]/lq,  NaiveFlag, &Stemp) * lq;
			S[1] = Stemp * lq;
			maximum = fmax(maximum, C[1]);
			maximum = fmax(maximum, Smax);
			if (ErrorFlag == 1) {
				ErrorFlag = 0;
				continue;
			}
			if (*T > (maximum + invprob) && Smax <= spaceBound) {
				//printf("%f\n", maximum+invprob);
				*T = maximum + invprob;
				MINvalues[0]=k; MINvalues[1]=w; // for d=3: 0 1
				for(int i=1; i<d; i++) MINvalues[i+1]=ww[i][0]; // 2 3
				for(int i=1; i<d; i++) MINvalues[d+i]=l[i]; // 4 5
				for(int i=1; i<d; i++) MINvalues[2*d+i-1]=r[i];     // 6 7
				for(int i=1; i<d; i++) MINvalues[3*d+i-2]=ww[i][i];   // 8 9
				for(int i=2; i<d; i++) MINvalues[4*d+i-4]=ww[i][i-1];   // 10
				for(int i=0; i<=d; i++) MINvalues[5*d+i-4]=p[i]; // 11 12 13 14
				for(int i=1; i<=d; i++) MINvalues[6*d+i-4]=S[i]; // 15 16 17
				for(int i=1; i<=d; i++) MINvalues[7*d+i-4]=C[i]; // 18 19 20
				MINvalues[8*d-3]= *T; // 21
				MINvalues[8*d-2] = invprob; // 22
				
				MINParams[0] = p[0];
				for (int i=1; i<d; i++) MINParams[i] = ww[i][0];
				for (int i=1; i<d; i++) MINParams[d+i-1] = l[i];
				for (int i=1; i<d; i++) MINParams[2*d-2+i] = eps[i];
			}
			
			eps[d] = epsswap;
		}
		}
		}
		}
		l[d] = lswap;
	}
	}
	}
	}
	}
	}
	}
	}
    }
    
	//printf("\n%d ", opti_steps);
    return 0.0;
    
}