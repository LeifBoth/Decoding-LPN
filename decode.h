double Prange(double k, double w, int HD_flag, double H1[], double MINvalues[32], double *T) {
	if (w==0) {
        w = inverse(H1, 1-k);
		if (HD_flag==1) w /= 2;
    }
	*T = H(H1,w) - H(H1,w/(1-k))*(1-k);
	
	MINvalues[0]=k; MINvalues[1]=w;
}
 
double BJMMPlus(double k, double w, int d, int HD_flag, int NN_flag, int Naive_flag, double pmin, double pmax, double psteps, double lmin, double lmax, double lsteps, double emin[6], double emax[6], double esteps[6], double H1[], double MINvalues[32], double MINParams[20], double *T) {
     
    if (w==0) {
        w = inverse(H1, 1-k);
        if (HD_flag) w /= 2;
    }
    double S[6], C[6], Ttemp, Stemp;
    double maximum, Smax;
    double l,invprob;
    double p[6], eps[6], r[6], epsswap;
	int check_c;
    *T = 1.0;
 
	for (p[0]=pmin; p[0]<=pmax+psteps*0.01; p[0] = p[0] + psteps) {
	for (l=fmin(lmax,1-k); l>=lmin-lsteps*0.01; l-=lsteps) {
		invprob = H(H1,w) - H(H1,p[0]/(k+l)) * (k+l) - H(H1,(w-p[0])/(1-k-l)) * (1-k-l);
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
				r[i] = p[i-1] + H(H1,eps[i]/(k+l-p[i-1])) * (k+l-p[i-1]);
				S[i] = H(H1,p[i]/(k+l)) * (k+l) - r[i];
				C[i] = fmax(0, 2*S[i] + r[i] - r[i-1]);
				if (p[i]/(k+l) > 1 || eps[i]/(k+l-p[i-1]) > 1 || k+l-p[i-1] < 0 || S[i] < 0) {
					check_c = 1;
					break;
				}
				if ((i==1) && (NN_flag==1)) {
					Stemp = S[i]/(1-k-l);
					Ttemp = NN((w-p[i-1])/(1-k-l), S[i]/(1-k-l), H1, Naive_flag, &Stemp) * (1-k-l);
					if (C[i] > Ttemp) {
						C[i] = Ttemp;
						S[i] = Stemp*(1-k-l);
					}
				}
			}
			if (check_c) continue;
			
			if (l >= r[1]) {
				S[d] = H(H1,2*p[d]/(k+l))*(k+l)/2;
				C[d] = fmax(0, 2*S[d] + r[d] - r[d-1]);
				if (GLOBAL_STOP==1) {
					GLOBAL_STOP=0;
					continue;
				}
				maximum = 0.0;
				Smax = 0.0;
				for (int i=1; i<=d; i++) {Smax = fmax(Smax, S[i]);}
				for (int i=1; i<=d; i++) {maximum = fmax(maximum, C[i]);}
				maximum = fmax(maximum,Smax);
				if (invprob >= 0) {
					if (*T > (maximum + invprob)) {
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
				} else printf("Invprob<0!!!!\n");
			}
			eps[d] = epsswap;
		}
		}
		}
	}
    }
 
    return 0.0;
 
}
	
double NewV3(double k, double w, int d, int HD_flag, int Naive_flag, double pmin, double pmax, double psteps, double lmin[6], double lmax[6], double lsteps[6], double emin[6], double emax[6], double esteps[6], double dwmin[6][6], double dwmax[6][6], double dwsteps[6][6], double wwmin[6], double wwmax[6], double wwsteps[6], double H1[], double MINvalues[32], double MINParams[20], double *T) {
    
    if (w==0) {
        w = inverse(H1, 1-k);
		if (HD_flag==1) w /= 2;
    }
    int check_c;
    double S[6], C[6], Stemp;
    double maximum, invprob;
    double l[7], p[6], eps[6], r[6], dw[6][6], ww[6], epsswap, lswap, dwsav, wq, lq, D[6];
    *T = 1.0;

    for (p[0]=pmin; p[0]<=pmax+psteps*0.01; p[0] = p[0] + psteps) {
		//printf("-- p[0]: %4f \n", p[0]);
	for (ww[1]=wwmin[1]; ww[1]<=fmin(wwmax[1],w-p[0])+wwsteps[1]*0.01; ww[1]+=wwsteps[1]) {
	for (ww[2]=wwmin[2]; ww[2]<=fmin(wwmax[2],w-p[0]-ww[1])+wwsteps[2]*0.01; ww[2]+=wwsteps[2]) {
	for (ww[3]=wwmin[3]; ww[3]<=fmin(wwmax[3],w-p[0]-ww[1]-ww[2])+wwsteps[3]*0.01; ww[3]+=wwsteps[3]) {
	for (dw[3][2]=fmax(dwmin[3][2],ww[3]/2); dw[3][2]<=dwmax[3][2]+dwsteps[3][2]*0.01; dw[3][2]+=dwsteps[3][2]) {
	for (l[1]=fmin(lmax[1],1-k); l[1]>=lmin[1]-lsteps[1]*0.01; l[1]-=lsteps[1]) {
	for (l[2]=fmin(lmax[2],l[1]); l[2]>=lmin[2]-lsteps[2]*0.01; l[2]-=lsteps[2]) {
	for (l[3]=fmin(lmax[3],l[1]-l[2]); l[3]>=lmin[3]-lsteps[3]*0.01; l[3]-=lsteps[3]) {
		lswap=l[d]; l[d] = 0.0;
		
		wq = w-p[0];
		lq = 1-k-l[1];
		for (int i=1; i<d; i++) {
			wq -= ww[i];
		}
		
		check_c = 0;
		for (int i=1; i<d; i++) {
			if (l[i]-l[i+1] == 0) continue;
			if (l[i]-l[i+1] < 0 || ww[i]/(l[i]-l[i+1]) < 0 || ww[i]/(l[i]-l[i+1]) > 1) {
				check_c = 1;
				break;
			}
		}
		if (check_c || wq/lq < 0 || wq/lq > 1) {
			continue;
		}
		invprob = H(H1,w) - H(H1,p[0]/k) * k - H(H1,wq/lq) * lq;
		for (int i=1; i<d; i++) {
			if (l[i]-l[i+1] == 0) continue;
			invprob -= H(H1,ww[i]/(l[i]-l[i+1])) * (l[i]-l[i+1]);
		}

		for (eps[1]=emin[1]; eps[1]<=emax[1]+esteps[1]*0.01; eps[1] = eps[1] + esteps[1]) {
		for (eps[2]=emin[2]; eps[2]<=emax[2]+esteps[2]*0.01; eps[2] = eps[2] + esteps[2]) {
		for (eps[3]=emin[3]; eps[3]<=emax[3]+esteps[3]*0.01; eps[3] = eps[3] + esteps[3]) {
			epsswap=eps[d], eps[d] = 0.0;
			
			for (int i=1; i<=d; i++) {
				p[i] = p[i-1]/2 + eps[i];
			}
			check_c = 0;
			for (int i=1; i<d; i++) {
				r[i] = p[i-1] + H(H1,eps[i]/(k-p[i-1])) * (k-p[i-1]);
				if (eps[i]/(k-p[i-1]) > 1 || k-p[i-1] < 0) {
					check_c = 1;
					break;
				}
			}
			if (check_c) continue;
			
			// only for m=2,3,4
			if (d==4) {
				dw[3][1] = l[3]-r[3];
				dw[3][0] = dw[3][1]/2;
				dw[2][1] = l[2]-r[2]-dw[3][2]
				         - (l[3]-dw[3][2])*H(H1, (dw[3][1]-dw[3][2]/2)/(l[3]-dw[3][2]));
				dw[2][0] = dw[2][1]/2;
				dwsav = (r[1]-l[1]+ww[1]+ww[2]+ww[3]
				      + (l[2]-l[3]-ww[2])*H(H1,(dw[2][1]-ww[2]/2)/(l[2]-l[3]-ww[2]))
					  + (l[3]-ww[3])*H(H1,(dw[3][2]-ww[3]/2)/(l[3]-ww[3])))
					  / (ww[1]-l[1]+l[2]);
				if (dwsav<0 || dwsav>1) {
					continue;
				}
				dw[1][0] = inverse(H1, dwsav) * (l[1]-l[2]-ww[1]) + ww[1]/2;
			} else if (d==3) {
				dw[2][1] = l[2]-r[2];
				dw[2][0] = dw[2][1]/2;
				dwsav = (r[1]-l[1]+ww[1]+ww[2]
				      + (l[2]-ww[2])*H(H1,(dw[2][1]-ww[2]/2)/(l[2]-ww[2])))
					  / (ww[1]-l[1]+l[2]);
				if (dwsav<0 || dwsav>1) {
					continue;
				}
				dw[1][0] = inverse(H1, dwsav) * (l[1]-l[2]-ww[1]) + ww[1]/2;
			} else if (d==2) {
				dwsav = (r[1]-l[1]+ww[1])/(ww[1]-l[1]);
				if (dwsav<0 || dwsav>1) {
					continue;
				}
				dw[1][0] = inverse(H1, dwsav) * (l[1]-ww[1]) + ww[1]/2;
			} else printf("Wrong depth! \n");

			S[d] = H(H1,2*p[d]/k) * k/2;
			S[d-1] = H(H1,p[d-1]/k) * k + (H(H1,dw[d-1][0]/l[d-1]) - 1) * l[d-1];
			if (S[d-1] < 0) continue;
			check_c = 0;
			for (int i=d-2; i>=1; i--) {
				if (dw[i][0]/(l[i]-l[i+1])<0 || dw[i][0]/(l[i]-l[i+1])>1) {
					check_c = 1;
					break;
				}
				S[i] = H(H1,p[i]/k) * k + (H(H1,dw[i][0]/(l[i]-l[i+1])) - 1) * (l[i]-l[i+1]);
				for (int j=i+1; j<d ;j++) {
					if (dw[j][j-i-1] == 0.0 && dw[j][j-i] == 0.0) continue;
				if (1-dw[j][j-i]/dw[j][j-i-1]/2 < 0 || 1-dw[j][j-i]/dw[j][j-i-1]/2 > 1 || dw[j][j-i]/2/(l[j]-l[j+1]-dw[j][j-i-1]) < 0 || dw[j][j-i]/2/(l[j]-l[j+1]-dw[j][j-i-1]) > 1 || dw[j][j-i-1]/(l[j]-l[j+1]) < 0 || dw[j][j-i-1]/(l[j]-l[j+1]) > 1) {
						check_c = 1;
						break;
					}
					S[i] += H(H1,1-dw[j][j-i]/dw[j][j-i-1]/2) * dw[j][j-i-1] 
					      + H(H1,dw[j][j-i]/2/(l[j]-l[j+1]-dw[j][j-i-1])) * (l[j]-l[j+1]-dw[j][j-i-1]) 
						  - H(H1,dw[j][j-i-1]/(l[j]-l[j+1])) * (l[j]-l[j+1]);
				}
				if (!check_c && S[i]>0) continue;
				check_c = 1;
				break;
			}
			if (check_c) continue;
			
			for (int i=2; i<=d; i++) {
				if (l[i-1]-l[i] == 0) {
					C[i] = 2*S[i];
				} else {
					Stemp = S[i]/(l[i-1]-l[i]);
					C[i] = NN(dw[i-1][0]/(l[i-1]-l[i]), S[i]/(l[i-1]-l[i]), H1, Naive_flag, &Stemp) * (l[i-1]-l[i]);
					S[i] = Stemp * (l[i-1]-l[i]);
				}
				if (i>2) continue;
			}
			Stemp = S[1]/lq;
			C[1] = NN(wq/lq, S[1]/lq, H1, Naive_flag, &Stemp) * lq;
			S[1] = Stemp * lq;
			
			if (GLOBAL_STOP==1) {
				GLOBAL_STOP=0;
				continue;
			}
			maximum = 0.0;
			for (int i=1; i<=d; i++) {maximum = fmax(fmax(maximum, S[i]),C[i]);}
			if (*T > (maximum + invprob)) {
				*T = maximum + invprob;
				// printf("depth=%d k=%.3f w=%f p=%.4f ", d, k, w, p[0]);
				// for(int i=1; i<d; i++) printf("w[%d]=%.4f ", i, ww[i]);
				// for(int i=1; i<d; i++) {
					// for(int j=0; j<i; j++) {
						// printf("dw[%d][%d]=%.6f ", i, j, dw[i][j]);
					// }
				// }
				// for(int i=1; i<d; i++) printf("l[%d]=%.4f ", i, l[i]);
				// for(int i=1; i<d; i++) printf("eps[%d]=%.4f ", i, eps[i]);
				// printf("T=%.5f \n", *T);
				MINvalues[0]=k; MINvalues[1]=w; // for d=3: 0 1
				for(int i=1; i<d; i++) MINvalues[i+1]=ww[i]; // 2 3
				for(int i=1; i<d; i++) MINvalues[d+i]=l[i]; // 4 5
				for(int i=1; i<d; i++) MINvalues[2*d+i-1]=r[i];     // 6 7
				for(int i=1; i<d; i++) MINvalues[3*d+i-2]=dw[i][0];   // 8 9
				for(int i=2; i<d; i++) MINvalues[4*d+i-4]=dw[i][1];   // 10
				for(int i=0; i<=d; i++) MINvalues[5*d+i-4]=p[i]; // 11 12 13 14
				for(int i=1; i<=d; i++) MINvalues[6*d+i-4]=S[i]; // 15 16 17
				for(int i=1; i<=d; i++) MINvalues[7*d+i-4]=C[i]; // 18 19 20
				MINvalues[8*d-3]= *T; // 21
				MINvalues[8*d-2] = invprob; // 22
				
				MINParams[0] = p[0];
				MINParams[1] = ww[1];
				MINParams[2] = ww[2];
				MINParams[3] = ww[3];
				MINParams[4] = l[1];
				MINParams[5] = l[2];
				MINParams[6] = l[3];
				MINParams[7] = eps[1];
				MINParams[8] = eps[2];
				MINParams[9] = eps[3];
				MINParams[10] = dw[3][2];
			}
			
			eps[d] = epsswap;
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
    
    return 0.0;
    
}

// old
double NewV2(double k, double w, int depth, int HD_flag, int Naive_flag, double pmin, double pmax, double psteps, double lmin[6], double lmax[6], double lsteps[6], double emin[6], double emax[6], double esteps[6], double dwmin[6][6], double dwmax[6][6], double dwsteps[6][6], double wwmin[6], double wwmax[6], double wwsteps[6], double H1[], double MINvalues[32], double MINParams[20], double *T) {
    
    if (w==0) {
        w = inverse(H1, 1-k);
		if (HD_flag==1) w /= 2;
    }
    int dmin=2, dmax=5, dsteps=1;
    if (depth>0) {
        dmin=depth;
        dmax=depth;
    }
    int d = depth, check_c;
    double S[6], C[6], Stemp;
    double maximum, invprob;
    double l[7], p[6], eps[6], r[6], dw[6][6], ww[6], epsswap, lswap, dwsav, wq, lq, D[6];
    *T = 1.0;

    for (p[0]=pmin; p[0]<=pmax+psteps*0.01; p[0] = p[0] + psteps) {
		//printf("-- p[0]: %4f \n", p[0]);
	for (ww[1]=wwmin[1]; ww[1]<=fmin(wwmax[1],w-p[0])+wwsteps[1]*0.01; ww[1]+=wwsteps[1]) {
	for (ww[2]=wwmin[2]; ww[2]<=fmin(wwmax[2],w-p[0]-ww[1])+wwsteps[2]*0.01; ww[2]+=wwsteps[2]) {
	for (ww[3]=wwmin[3]; ww[3]<=fmin(wwmax[3],w-p[0]-ww[1]-ww[2])+wwsteps[3]*0.01; ww[3]+=wwsteps[3]) {
	for (dw[2][1]=fmax(dwmin[2][1],ww[2]/2); dw[2][1]<=dwmax[2][1]+dwsteps[2][1]*0.01; dw[2][1]+=dwsteps[2][1]) {
	for (dw[3][2]=fmax(dwmin[3][2],ww[3]/2); dw[3][2]<=dwmax[3][2]+dwsteps[3][2]*0.01; dw[3][2]+=dwsteps[3][2]) {
	for (dw[3][1]=fmax(dwmin[3][1],dw[3][2]/2); dw[3][1]<=dwmax[3][1]+dwsteps[3][1]*0.01; dw[3][1]+=dwsteps[3][1]) {
	for (l[1]=fmin(lmax[1],1-k); l[1]>=lmin[1]-lsteps[1]*0.01; l[1]-=lsteps[1]) {
	for (l[2]=fmin(lmax[2],l[1]); l[2]>=lmin[2]-lsteps[2]*0.01; l[2]-=lsteps[2]) {
	for (l[3]=fmin(lmax[3],l[1]-l[2]); l[3]>=lmin[3]-lsteps[3]*0.01; l[3]-=lsteps[3]) {
		lswap=l[d]; l[d] = 0.0;
		
		wq = w-p[0];
		lq = 1-k-l[1];
		for (int i=1; i<d; i++) {
			wq -= ww[i];
		}
		
		check_c = 0;
		for (int i=1; i<d; i++) {
			if (l[i]-l[i+1] == 0) continue;
			if (l[i]-l[i+1] < 0 || ww[i]/(l[i]-l[i+1]) < 0 || ww[i]/(l[i]-l[i+1]) > 1) {
				check_c = 1;
				break;
			}
		}
		if (check_c || wq/lq < 0 || wq/lq > 1) {
			continue;
		}
		invprob = H(H1,w) - H(H1,p[0]/k) * k - H(H1,wq/lq) * lq;
		for (int i=1; i<d; i++) {
			if (l[i]-l[i+1] == 0) continue;
			invprob -= H(H1,ww[i]/(l[i]-l[i+1])) * (l[i]-l[i+1]);
		}

		for (eps[1]=emin[1]; eps[1]<=emax[1]+esteps[1]*0.01; eps[1] = eps[1] + esteps[1]) {
		for (eps[2]=emin[2]; eps[2]<=emax[2]+esteps[2]*0.01; eps[2] = eps[2] + esteps[2]) {
		for (eps[3]=emin[3]; eps[3]<=emax[3]+esteps[3]*0.01; eps[3] = eps[3] + esteps[3]) {
			epsswap=eps[d], eps[d] = 0.0;
			
			for (int i=1; i<=d; i++) {
				p[i] = p[i-1]/2 + eps[i];
			}
			for (int i=1; i<d; i++) {
				r[i] = p[i-1] + H(H1,eps[i]/(k-p[i-1])) * (k-p[i-1]);
			}
			
			// only for m=2,3,4
			if (d==4) {
				dwsav = (r[3]-l[3]+dw[3][1])/(dw[3][1]-l[3]);
				if (dwsav<0 || dwsav>1) {
					continue;
				}
				dw[3][0] = inverse(H1, dwsav) * (l[3]-dw[3][1]) + dw[3][1]/2;
				dwsav = (r[2]-l[2]+dw[2][1]+dw[3][2]
				      + (l[3]-dw[3][2])*H(H1,(dw[3][1]-dw[3][2]/2)/(l[3]-dw[3][2])))
					  / (dw[2][1]-l[2]+l[3]);
				if (dwsav<0 || dwsav>1) {
					continue;
				}
				dw[2][0] = inverse(H1, dwsav) * (l[2]-l[3]-dw[2][1]) + dw[2][1]/2;
				dwsav = (r[1]-l[1]+ww[1]+ww[2]+ww[3]
				      + (l[2]-l[3]-ww[2])*H(H1,(dw[2][1]-ww[2]/2)/(l[2]-l[3]-ww[2]))
					  + (l[3]-ww[3])*H(H1,(dw[3][2]-ww[3]/2)/(l[3]-ww[3])))
					  / (ww[1]-l[1]+l[2]);	
				if (dwsav<0 || dwsav>1) {
					continue;
				}
				dw[1][0] = inverse(H1, dwsav) * (l[1]-l[2]-ww[1]) + ww[1]/2;
			} else if (d==3) {
				dwsav = (r[2]-l[2]+dw[2][1])/(dw[2][1]-l[2]);
				if (dwsav<0 || dwsav>1) {
					continue;
				}
				dw[2][0] = inverse(H1, dwsav) * (l[2]-dw[2][1]) + dw[2][1]/2;
				dwsav = (r[1]-l[1]+ww[1]+ww[2]
				      + (l[2]-ww[2])*H(H1,(dw[2][1]-ww[2]/2)/(l[2]-ww[2])))
					  / (ww[1]-l[1]+l[2]);
				if (dwsav<0 || dwsav>1) {
					continue;
				}
				dw[1][0] = inverse(H1, dwsav) * (l[1]-l[2]-ww[1]) + ww[1]/2;
			} else if (d==2) {
				dwsav = (r[1]-l[1]+ww[1])/(ww[1]-l[1]);
				if (dwsav<0 || dwsav>1) {
					continue;
				}
				dw[1][0] = inverse(H1, dwsav) * (l[1]-ww[1]) + ww[1]/2;
			} else printf("Wrong depth! \n");
			
			S[d] = H(H1,2*p[d]/k) * k/2;
			S[d-1] = H(H1,p[d-1]/k) * k + (H(H1,dw[d-1][0]/l[d-1]) - 1) * l[d-1];
			if (S[d-1] < 0) continue;
			check_c = 0;
			for (int i=d-2; i>=1; i--) {
				if (dw[i][0]/(l[i]-l[i+1])<0 || dw[i][0]/(l[i]-l[i+1])>1) {
					check_c = 1;
					break;
				}
				S[i] = H(H1,p[i]/k) * k + (H(H1,dw[i][0]/(l[i]-l[i+1])) - 1) * (l[i]-l[i+1]);
				for (int j=i+1; j<d ;j++) {
					if (dw[j][j-i-1] == 0.0 && dw[j][j-i] == 0.0) continue;
				if (1-dw[j][j-i]/dw[j][j-i-1]/2 < 0 || 1-dw[j][j-i]/dw[j][j-i-1]/2 > 1 || dw[j][j-i]/2/(l[j]-l[j+1]-dw[j][j-i-1]) < 0 || dw[j][j-i]/2/(l[j]-l[j+1]-dw[j][j-i-1]) > 1 || dw[j][j-i-1]/(l[j]-l[j+1]) < 0 || dw[j][j-i-1]/(l[j]-l[j+1]) > 1) {
						check_c = 1;
						break;
					}
					S[i] += H(H1,1-dw[j][j-i]/dw[j][j-i-1]/2) * dw[j][j-i-1] 
					      + H(H1,dw[j][j-i]/2/(l[j]-l[j+1]-dw[j][j-i-1])) * (l[j]-l[j+1]-dw[j][j-i-1]) 
						  - H(H1,dw[j][j-i-1]/(l[j]-l[j+1])) * (l[j]-l[j+1]);
				}
				if (!check_c && S[i]>0) continue;
				check_c = 1;
				break;
			}
			if (check_c) continue;
			
			for (int i=2; i<=d; i++) {
				if (l[i-1]-l[i] == 0) {
					C[i] = 2*S[i];
				} else {
					Stemp = S[i]/(l[i-1]-l[i]);
					C[i] = NN(dw[i-1][0]/(l[i-1]-l[i]), S[i]/(l[i-1]-l[i]), H1, Naive_flag, &Stemp) * (l[i-1]-l[i]);
					S[i] = Stemp*(l[i-1]-l[i]);
				}
				if (i>2) continue;
			}
			Stemp = S[1]/lq;
			C[1] = NN(wq/lq, S[1]/lq, H1, Naive_flag, &Stemp) * lq;
			S[1] = Stemp * lq;
			
			maximum = 0.0;
			for (int i=1; i<=d; i++) {maximum = fmax(fmax(maximum, S[i]),C[i]);}
			if (*T > (maximum + invprob)) {
				*T = maximum + invprob;
				// printf("depth=%d k=%.3f w=%f p=%.4f ", d, k, w, p[0]);
				// for(int i=1; i<d; i++) printf("w[%d]=%.4f ", i, ww[i]);
				// for(int i=1; i<d; i++) {
					// for(int j=0; j<i; j++) {
						// printf("dw[%d][%d]=%.6f ", i, j, dw[i][j]);
					// }
				// }
				// for(int i=1; i<d; i++) printf("l[%d]=%.4f ", i, l[i]);
				// for(int i=1; i<d; i++) printf("eps[%d]=%.4f ", i, eps[i]);
				// printf("T=%.5f \n", *T);
				MINvalues[0]=k; MINvalues[1]=w; // for d=3: 0 1
				for(int i=1; i<d; i++) MINvalues[i+1]=ww[i]; // 2 3
				for(int i=1; i<d; i++) MINvalues[d+i]=l[i]; // 4 5
				for(int i=1; i<d; i++) MINvalues[2*d+i-1]=r[i];     // 6 7
				for(int i=1; i<d; i++) MINvalues[3*d+i-2]=dw[i][0];   // 8 9
				for(int i=2; i<d; i++) MINvalues[4*d+i-4]=dw[i][1];   // 10
				for(int i=0; i<=d; i++) MINvalues[5*d+i-4]=p[i]; // 11 12 13 14
				for(int i=1; i<=d; i++) MINvalues[6*d+i-4]=S[i]; // 15 16 17
				for(int i=1; i<=d; i++) MINvalues[7*d+i-4]=C[i]; // 18 19 20
				MINvalues[8*d-3]= *T; // 21
				MINvalues[8*d-2] = invprob; // 22
				
				MINParams[0] = p[0];
				MINParams[1] = ww[1];
				MINParams[2] = ww[2];
				MINParams[3] = ww[3];
				MINParams[4] = l[1];
				MINParams[5] = l[2];
				MINParams[6] = l[3];
				MINParams[7] = eps[1];
				MINParams[8] = eps[2];
				MINParams[9] = eps[3];
				MINParams[10] = dw[3][2];
				MINParams[11] = dw[3][1];
				MINParams[12] = dw[2][1];
			}
			
			eps[d] = epsswap;
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
	}
    
    return 0.0;
    
}