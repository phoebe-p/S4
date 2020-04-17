/* Copyright (C) 2009-2011, Stanford University
 * This file is part of S4
 * Written by Victor Liu (vkl@stanford.edu)
 *
 * S4 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * S4 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <config.h>

#include <cmath>
#include <S4.h>
#include "../RNP/TBLAS.h"
#ifdef HAVE_BLAS
# include "../RNP/TBLAS_ext.h"
#endif
#include "../RNP/LinearSolve.h"
#ifdef HAVE_LAPACK
# include "../RNP/LinearSolve_lapack.h"
#endif
#include "fmm.h"
#ifdef DUMP_MATRICES
# define RNP_OUTPUT_MATHEMATICA
# define DUMP_STREAM std::cerr
//# define DUMP_STREAM (omega.real() > 1.91637 ? std::cerr : std::cout)
# include <IO.h>
#endif

#include <iostream>
#include <limits>
//#include <kiss_fft.h>
//#include <tools/kiss_fftnd.h>
#include <cstdio>
#include "fft_iface.h"
#include <cstring>

int FMMGetEpsilon_PolBasisVL(const Simulation *S, const Layer *L, const int n, std::complex<double> *Epsilon2, std::complex<double> *Epsilon_inv){
	double mp1 = 0;
	int pwr = S->options.lanczos_smoothing_power;
	if(S->options.use_Lanczos_smoothing){
		mp1 = GetLanczosSmoothingOrder(S);
		S4_TRACE("I   Lanczos smoothing order = %f\n", mp1);
		mp1 *= S->options.lanczos_smoothing_width;
	}
	if(Epsilon_inv){} // prevent unused parameter warning
	
	const int n2 = 2*n;
	const int nn = n*n;
	const double unit_cell_size = Simulation_GetUnitCellSize(S);
	const int *G = S->solution->G;
    // Number of dimensions is 1 if 2nd component of first lattice vector is
    // zero and first component of 2nd lattice vector is 0 in real space,
    // otherwise its 2D
	const int ndim = (0 == S->Lr[2] && 0 == S->Lr[3]) ? 1 : 2;
	double *ivalues = (double*)S4_malloc(sizeof(double)*(2+10)*(L->pattern.nshapes+1));
	double *values = ivalues + 2*(L->pattern.nshapes+1);
	
	// Get all the dielectric tensors
	//bool have_tensor = false;
	for(int i = -1; i < L->pattern.nshapes; ++i){
		const Material *M;
		if(-1 == i){
			M = Simulation_GetMaterialByName(S, L->material, NULL);
		}else{
			M = Simulation_GetMaterialByIndex(S, L->pattern.shapes[i].tag);
		}
		if(0 == M->type){
			std::complex<double> eps_temp(M->eps.s[0], M->eps.s[1]);
			//eps_temp = Simulation_GetEpsilonByIndex(S, L->pattern.shapes[i].tag);
			values[2*(i+1)+0] = eps_temp.real();
			values[2*(i+1)+1] = eps_temp.imag();
			eps_temp = 1./eps_temp;
			ivalues[2*(i+1)+0] = eps_temp.real();
			ivalues[2*(i+1)+1] = eps_temp.imag();
		}else{
			//have_tensor = true;
		}
	}
	
	// Epsilon2 is
	//   [ Epsilon - Delta*Pxx        -Delta*Pxy     ]
	//   [     -Delta*Pyx        Epsilon - Delta*Pyy ]
	// Pxy = Fourier transform of par_x^* par_y
	//
	// Need temp storage for Delta and P__
	
	std::complex<double> *P = Simulation_GetCachedField(S, L);
	std::complex<double> *W = Simulation_GetCachedW(S, L);
	std::complex<double> *work = NULL;
	std::complex<double> *mDelta = NULL;
	std::complex<double> *Eta = NULL;
    int pnotcached = NULL == P;
    int ng2;
    double ing2;
	if(pnotcached){
		// We need to compute the vector field

		// Make vector fields
		// Determine size of the vector field grid
		int ngrid[2] = {1,1};
		for(int i = 0; i < 2; ++i){ // choose grid size
			int allzero = 1;
			for(int j = 0; j < n; ++j){
				if(abs(G[2*j+i]) > ngrid[i]){ ngrid[i] = abs(G[2*j+i]); }
				if(0 != G[2*j+i]){ allzero = 0; }
			}
			if(allzero){
				ngrid[i] = 1;
			}else{
				if(ngrid[i] < 1){ ngrid[i] = 1; }
				ngrid[i] *= S->options.resolution;
				ngrid[i] = fft_next_fast_size(ngrid[i]);
			}
		}
        // Number of real space grid points
		ng2 = ngrid[0]*ngrid[1];
        /* std::cout << "ngrid[0]: " << ngrid[0] << std::endl; */
        /* std::cout << "ngrid[1]: " << ngrid[1] << std::endl; */
	
        // Memory workspace array. N = number of basis terms, ng = number of
        // real space grid points for vector field. Has length 6N^2 + 4*ng 
		work = (std::complex<double>*)S4_malloc(sizeof(std::complex<double>)*(6*nn + 4*ng2));
        // mDelta stored from beginning of workspace to N^2
        // mDelta is an NxN matrix
        // mDelta = work[0:N^2]
		mDelta = work;
        // Eta is        // Eta = work[N^2:2N^2]
		Eta = mDelta + nn;
        // P is an NxN matrix in real space, which is why we only allocate N^2
        // elements for it here. After Fourier transforming it becomes larger
        // P = work[2N^2:3N^2]
		P = Eta + nn;
        // Ffrom is the real space thing we will be Fourier transforming
        // Ffrom = work[3N^2:7N^2]
		std::complex<double> *Ffrom = P + 4*nn; // Fourier source
        // This is where the results of the fourier transform are placed.
        // Ffrom = work[7N^2:7N^2 + ng]
		std::complex<double> *Fto = Ffrom + ng2; // Fourier dest
        // The real space vector field. 
        // par = work[7N^2 + ng: 7N^2 + 2ng]
		std::complex<double> *par = Fto + ng2; // real space parallel vector

		// Generate the vector field
		ing2 = 1./(double)ng2;
		int ii[2];
	
        // Creates workspace for the vector field. The vector field in general
        // has two components, so we need to store two doubles for every grid
        // point
        double *vfield = (double*)S4_malloc(sizeof(double)*2*ng2);
		if(0 == S->Lr[2] && 0 == S->Lr[3]){ // 1D, generate the trivial field
			double nv[2] = {-S->Lr[1], S->Lr[0]};
			double nva = hypot(nv[0],nv[1]);
			nv[0] /= nva; nv[1] /= nva;
			for(ii[1] = 0; ii[1] < ngrid[1]; ++ii[1]){
				for(ii[0] = 0; ii[0] < ngrid[0]; ++ii[0]){
					vfield[2*(ii[0]+ii[1]*ngrid[0])+0] = nv[0];
					vfield[2*(ii[0]+ii[1]*ngrid[0])+1] = nv[1];
				}
			}
		}else{
			S4_VERB(1, "Generating polarization vector field of size %d x %d\n", ngrid[0], ngrid[1]);
            // 2nd argument here determines the type of vector field. 0 means
            // tangential, 1 means normal. Normal vector fields are currently
            // broken. This puts the vector field for the patterning of his
            // layer into the vfield array
            // Lr is the array containing the real space lattice vectors
			int error = Pattern_GenerateFlowField(&L->pattern, 0, S->Lr, ngrid[0], ngrid[1], vfield);
			
			if(0 != error){
				S4_TRACE("< Simulation_ComputeLayerBands (failed; Pattern_GenerateFlowField returned %d) [omega=%f]\n", error, S->omega[0]);
				if(NULL != vfield){ S4_free(vfield); }
				if(NULL != work){ S4_free(work); }
				if(NULL != ivalues){ S4_free(ivalues); }
				return error;
			}
			
			// Normalize the field to max length
			{
				double maxlen = 0;
				for(ii[1] = 0; ii[1] < ngrid[1]; ++ii[1]){
					for(ii[0] = 0; ii[0] < ngrid[0]; ++ii[0]){
						double a = hypot(
							vfield[2*(ii[0]+ii[1]*ngrid[0])+0],
							vfield[2*(ii[0]+ii[1]*ngrid[0])+1]);
						if(a > maxlen){
							maxlen = a;
						}
					}
				}
				maxlen = 1./maxlen;
				for(ii[1] = 0; ii[1] < ngrid[1]; ++ii[1]){
					for(ii[0] = 0; ii[0] < ngrid[0]; ++ii[0]){
						vfield[2*(ii[0]+ii[1]*ngrid[0])+0] *= maxlen;
						vfield[2*(ii[0]+ii[1]*ngrid[0])+1] *= maxlen;
					}
				}
			}
		}

		if(NULL != S->options.vector_field_dump_filename_prefix){
			const char *layer_name = NULL != L->name ? L->name : "";
			const size_t prefix_len = strlen(S->options.vector_field_dump_filename_prefix);
			char *filename = (char*)malloc(sizeof(char) * (prefix_len + strlen(layer_name) + 1));
			strcpy(filename, S->options.vector_field_dump_filename_prefix);
			strcpy(filename+prefix_len, layer_name);
			FILE *fp = fopen(filename, "wb");
			if(NULL != fp){
				for(ii[1] = 0; ii[1] < ngrid[1]; ++ii[1]){
					for(ii[0] = 0; ii[0] < ngrid[0]; ++ii[0]){
						fprintf(fp, "%d\t%d\t%f\t%f\n", ii[0], ii[1], vfield[2*(ii[0]+ii[1]*ngrid[0])+0], vfield[2*(ii[0]+ii[1]*ngrid[0])+1]);
					} fprintf(fp, "\n");
				}
				fclose(fp);
			}
			free(filename);
		}
			
	    // Here vfield contains the real space vector field. It seems to be
        // copied exactly into par. This is necessary because par is of complex
        // double type, while vfield is just of type double. 
        for(ii[1] = 0; ii[1] < ngrid[1]; ++ii[1]){
			for(ii[0] = 0; ii[0] < ngrid[0]; ++ii[0]){
				par[2*(ii[0]+ii[1]*ngrid[0])+0] = vfield[2*(ii[0]+ii[1]*ngrid[0])+0];
				par[2*(ii[0]+ii[1]*ngrid[0])+1] = vfield[2*(ii[0]+ii[1]*ngrid[0])+1];
			}
		}
		

		fft_plan plan = fft_plan_dft_2d(ngrid, Ffrom, Fto, 1);

		// We fill in the quarter blocks of F in Fortran order
		for(int w = 0; w < 4; ++w){
			int Erow = (w&1 ? n : 0);
			int Ecol = (w&2 ? n : 0);
			int _1 = (w&1);
			int _2 = ((w&2)>>1);
			for(ii[1] = 0; ii[1] < ngrid[1]; ++ii[1]){
				const int si1 = ii[1] >= ngrid[1]/2 ? ii[1]-ngrid[1]/2 : ii[1]+ngrid[1]/2;
				for(ii[0] = 0; ii[0] < ngrid[0]; ++ii[0]){
					const int si0 = ii[0] >= ngrid[0]/2 ? ii[0]-ngrid[0]/2 : ii[0]+ngrid[0]/2;
					Ffrom[si1+si0*ngrid[1]] = par[2*(ii[0]+ii[1]*ngrid[0])+_1]*par[2*(ii[0]+ii[1]*ngrid[0])+_2];
				}
			}
			fft_plan_exec(plan);

            // This is now copying the results of the Fourier transform (the
            // Fourier transform of P) into the P pointer. The P pointer is a
            // just a pointer to a particular location in the worksapce array.
            // P in real space in NxN, but its Fourier transform is 2Nx2N, so
            // here we are computing a larger matrix and just storing it in P.
            // This means we are actually overwriting parts of Ffrom, but
            // that's okay because we already Fourier transformed it and don't
            // need it anymore
			for(int j = 0; j < n; ++j){
				for(int i = 0; i < n; ++i){
                    // Get indices of lattice points in Fourier space
					int f[2] = {G[2*i+0]-G[2*j+0],G[2*i+1]-G[2*j+1]};
                    // Shift so everything its indexed from 0
					if(f[0] < 0){ f[0] += ngrid[0]; }
					if(f[1] < 0){ f[1] += ngrid[1]; }
					P[Erow+i+(Ecol+j)*n2] = ing2 * Fto[f[1]+f[0]*ngrid[1]];
				}
			}
		}
        // The x and y components of the vector field at each point are stored
        // adjacent to one another in vfield, 
		fft_plan_destroy(plan);

		if(NULL != vfield){ S4_free(vfield); }
	}else{
		// P contains the cached version
		// We still need temporary space to compute -Delta
		work = (std::complex<double>*)S4_malloc(sizeof(std::complex<double>)*2*nn);
		mDelta = work;
		Eta = mDelta + nn;
	}
	
	// Generate the Fourier matrix of epsilon^{-1}. See eqns 6,7,8 of paper
	for(int j = 0; j < n; ++j){
		for(int i = 0; i < n; ++i){
			int dG[2] = {G[2*i+0]-G[2*j+0],G[2*i+1]-G[2*j+1]};
			double f[2] = {
				dG[0] * S->Lk[0] + dG[1] * S->Lk[2],
				dG[0] * S->Lk[1] + dG[1] * S->Lk[3]
				};
			double sigma = 1.;
			if(S->options.use_Lanczos_smoothing){
				sigma = GetLanczosSmoothingFactor(mp1, pwr, f);
			}
			double ft[2];
			Pattern_GetFourierTransform(&L->pattern, ivalues, f, ndim, unit_cell_size, ft);
			Eta[i+j*n] = sigma * std::complex<double>(ft[0],ft[1]);
		}
	}

#ifdef DUMP_MATRICES
        DUMP_STREAM << "Eta:" << std::endl;
        RNP::IO::PrintMatrix(n,n,Eta,n, DUMP_STREAM) << std::endl << std::endl;
#endif
    if(NULL == W && pnotcached){
        // First construct eta_inv
        // Allocate memory for eta^-1 and set it to the identity
        std::complex<double> *eta_inv = (std::complex<double>*)S4_malloc(sizeof(std::complex<double>) * n*n);
        RNP::TBLAS::SetMatrix<'A'>(n,n, std::complex<double>(0.), std::complex<double>(1.), eta_inv, n);
        // We need to copy Eta so we don't muck it up later on
        std::complex<double> *Eta_cpy = (std::complex<double>*)S4_malloc(sizeof(std::complex<double>) * n*n);
		RNP::TBLAS::CopyMatrix<'A'>(n,n, Eta, n, Eta_cpy, n);
#ifdef DUMP_MATRICES
        DUMP_STREAM << "Eta_cpy:" << std::endl;
        RNP::IO::PrintMatrix(n,n,Eta_cpy,n, DUMP_STREAM) << std::endl << std::endl;
#endif
        // Compute it's inverse
        RNP::LinearSolve<'N'>(n,n,Eta_cpy,n, eta_inv,n, NULL, NULL);
        // Get rid of the copy
        S4_free(Eta_cpy);
#ifdef DUMP_MATRICES
            DUMP_STREAM << "eta_inv:" << std::endl;
            RNP::IO::PrintMatrix(n,n,eta_inv,n, DUMP_STREAM) << std::endl << std::endl;
#endif

        // Compute the thing inside the parenthesis in eqn 9a of Weismanns
        // paper (I call it W) and store it in the memory space of W
        // This is looping through blocks of the matrix P (2Nx2N) and
        // multiplying each sub-block by eta_inv (NxN) independently. w
        // indexes the block. 
        /* std::complex<double> *W; */
        W = (std::complex<double>*)S4_malloc(sizeof(std::complex<double>) * n2*n2);
        for(int w = 0; w < 4; ++w){
            // If w == 1, Erow = n, else Erow = 0
            // If w == 2, Ecol = n, else Ecol = 0
            // w = 0, Erow = 0 , Ecol = 0, top left block
            // w = 1, Erow = n, Ecol = 0, top right block
            // w = 2, Erow = 0, Ecol = n, bottom left block
            // w = 3, Erow = n, Ecol = n, bottom right block
            int Erow = (w&1 ? n : 0);
            int Ecol = (w&2 ? n : 0);
            // m = n, n = n, k = n
            // mDelta sub-block = A (nxn)
            // P sub-block = B (nxn)
            // Epsilon2 sub-block = C (nxn)
            // alpha = beta = 1
            // Computes C := alpha*A*B + beta*C
            // We'll do the first term first, which means right multiplying by Epsilon_inv. 
            // We don't add the contents of Ncombo here because its empty
            RNP::TBLAS::MultMM<'N','N'>(n,n,n, std::complex<double>(.5),&P[Erow+Ecol*n2],n2,eta_inv,n,std::complex<double>(0),&W[Erow+Ecol*n2],n2);
            // Now do the second term, which means left multiplying by
            // Epsilon_inv. Now we _do_ add the contents of Ncombo, bcause it
            // already contains the first term
            RNP::TBLAS::MultMM<'N','N'>(n,n,n, std::complex<double>(.5),eta_inv,n,&P[Erow+Ecol*n2],n2,std::complex<double>(1.),&W[Erow+Ecol*n2],n2);
        }
        
#ifdef DUMP_MATRICES
        /* DUMP_STREAM << "N*Epsilon2:" << std::endl; */
        /* RNP::IO::PrintMatrix(n2,n2,N,n2, DUMP_STREAM) << std::endl << std::endl; */
#endif
#ifdef DUMP_MATRICES
        DUMP_STREAM << "W:" << std::endl;
        RNP::IO::PrintMatrix(n2,n2,W,n2, DUMP_STREAM) << std::endl << std::endl;
#endif
		// Add to cache
		Simulation_AddFieldToCache((Simulation*)S, L, S->n_G, P, 4*nn, W, 4*nn);
        // Now free up all the memory we just allocated
        S4_free(eta_inv);
        S4_free(W);
    }
#ifdef DUMP_MATRICES
        DUMP_STREAM << "Eta:" << std::endl;
        RNP::IO::PrintMatrix(n,n,Eta,n, DUMP_STREAM) << std::endl << std::endl;
#endif
	// mDelta will contain -Delta = inv(Eta) - Epsilon
	// Epsilon2 still only has Epsilon along its diagonal
	RNP::TBLAS::SetMatrix<'A'>(n,n, 0.,1., mDelta,n);
    // I think this is just intended to invert Eta and put it along the
    // diagonal of mDelta. Recall Delta = epsilon - eta^-1 so 
    // -Delta = eta^-1 - epsilon. This seems to make sense because in the loop
    // we then subtract epsilon from mDelta
    // This is equivalent to backslash operator in MATLAB, which is different
    // from taking the inverse and multiplying. Can be faster and better
    // conditioned numerically with the same result
    // Basically taking the inverse by passing the identity into the fifth
    // argument and then sticking it into mDelta
	RNP::LinearSolve<'N'>(n,n, Eta,n, mDelta,n, NULL, NULL);
	for(int i = 0; i < n; ++i){
        // alpha = -1
        // x = Epsilon2
        // y = mDelta
        // Computes y := alpha*x + y
		RNP::TBLAS::Axpy(n, std::complex<double>(-1.), &Epsilon2[0+i*n2],1, &mDelta[0+i*n],1);
	}
    // This is looping through blocks of the matrix in equation 51 (i.e Epsilon
    // 2) and filling in each sub-block independently. w indexes the block
	for(int w = 0; w < 4; ++w){
        // If w == 1, Erow = n, else Erow = 0
        // If w == 2, Ecol = n, else Ecol = 0
        // w = 0, Erow = 0 , Ecol = 0, top left block
        // w = 1, Erow = n, Ecol = 0, top right block
        // w = 2, Erow = 0, Ecol = n, bottom left block
        // w = 3, Erow = n, Ecol = n, bottom right block
        int Erow = (w&1 ? n : 0);
		int Ecol = (w&2 ? n : 0);
        // m = n, n = n, k = n
        // mDelta sub-block = A (nxn)
        // P sub-block = B (nxn)
        // Epsilon2 sub-block = C (nxn)
        // alpha = beta = 1
        // Computes C := alpha*A*B + beta*C
		RNP::TBLAS::MultMM<'N','N'>(n,n,n, std::complex<double>(1.),mDelta,n, &P[Erow+Ecol*n2],n2, std::complex<double>(1.),&Epsilon2[Erow+Ecol*n2],n2);
	}
	if(NULL != work){ S4_free(work); }
	
	S4_free(ivalues);
	
	return 0;
}
