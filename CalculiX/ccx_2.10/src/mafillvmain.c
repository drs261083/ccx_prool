/*     CalculiX - A 3-dimensional finite element program                 */
/*              Copyright (C) 1998-2015 Guido Dhondt                          */

/*     This program is free software; you can redistribute it and/or     */
/*     modify it under the terms of the GNU General Public License as    */
/*     published by the Free Software Foundation(version 2);    */
/*                    */

/*     This program is distributed in the hope that it will be useful,   */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of    */ 
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the      */
/*     GNU General Public License for more details.                      */

/*     You should have received a copy of the GNU General Public License */
/*     along with this program; if not, write to the Free Software       */
/*     Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.         */

#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>
#include "CalculiX.h"

static char *lakonf1;

static ITG num_cpus,*nef1,*ipnei1,*neifa1,*neiel1,*jq1,*irow1,*nzs1,*ielfa1,*
    ifabou1,*nbody1,*neq1,*nactdohinv1;

static double *auv1=NULL,*adv1=NULL,*bv1=NULL,*vfa1,*xxn1,*area1,*vel1,
       *cosa1,*umfa1,*xlet1,*xle1,*gradvfa1,*xxi1,*body1,*volume1,*dtimef1,
       *velo1,*veloo1,*sel1,*xrlfa1,*gamma1,*xxj1,*a11,*a21,*a31,*flux1;

void mafillvmain(ITG *nef,ITG *ipnei,ITG *neifa,ITG *neiel,
             double *vfa,double *xxn,double *area,double *auv,double *adv,
             ITG *jq,ITG *irow,ITG *nzs,double *bv,double *vel,double *cosa,
             double *umfa,double *xlet,double *xle,double *gradvfa,
	     double *xxi,double *body,double *volume,
	     ITG *ielfa,char *lakonf,ITG *ifabou,ITG *nbody,ITG *neq,
	     double *dtimef,double *velo,double *veloo,
	     double *sel,double *xrlfa,double *gamma,double *xxj,
	     ITG *nactdohinv,double *a1,double *a2,double *a3,double *flux){

    ITG i,j;
      
    /* variables for multithreading procedure */
    
    ITG sys_cpus,*ithread=NULL;
    char *env,*envloc,*envsys;

    num_cpus = 0;
    sys_cpus=0;

    /* explicit user declaration prevails */

    envsys=getenv("NUMBER_OF_CPUS");
    if(envsys){
	sys_cpus=atoi(envsys);
	if(sys_cpus<0) sys_cpus=0;
    }

    /* automatic detection of available number of processors */

    if(sys_cpus==0){
	sys_cpus = getSystemCPUs();
	if(sys_cpus<1) sys_cpus=1;
    }

    /* local declaration prevails, if strictly positive */

    envloc = getenv("CCX_NPROC_CFD");
    if(envloc){
	num_cpus=atoi(envloc);
	if(num_cpus<0){
	    num_cpus=0;
	}else if(num_cpus>sys_cpus){
	    num_cpus=sys_cpus;
	}
	
    }

    /* else global declaration, if any, applies */

    env = getenv("OMP_NUM_THREADS");
    if(num_cpus==0){
	if (env)
	    num_cpus = atoi(env);
	if (num_cpus < 1) {
	    num_cpus=1;
	}else if(num_cpus>sys_cpus){
	    num_cpus=sys_cpus;
	}
    }

// next line is to be inserted in a similar way for all other paralell parts

    if(*nef<num_cpus) num_cpus=*nef;
    
    pthread_t tid[num_cpus];

    /* allocating fields for lhs and rhs matrix */

    NNEW(adv1,double,num_cpus**neq);
    NNEW(auv1,double,(long long)num_cpus*2**nzs);
    NNEW(bv1,double,num_cpus*3**neq);

    /* calculating the stiffness and/or mass matrix 
       (symmetric part) */

    nef1=nef;ipnei1=ipnei;neifa1=neifa;neiel1=neiel;vfa1=vfa;xxn1=xxn;
    area1=area;jq1=jq;irow1=irow;nzs1=nzs;vel1=vel;cosa1=cosa;umfa1=umfa;
    xlet1=xlet;xle1=xle;gradvfa1=gradvfa;xxi1=xxi;body1=body;volume1=volume;
    ielfa1=ielfa;lakonf1=lakonf;ifabou1=ifabou;nbody1=nbody;neq1=neq;
    dtimef1=dtimef;velo1=velo;veloo1=veloo;sel1=sel;xrlfa1=xrlfa;
    gamma1=gamma;xxj1=xxj;nactdohinv1=nactdohinv;a11=a1;a21=a2;a31=a3;
    flux1=flux;
    
    /* create threads and wait */
    
    NNEW(ithread,ITG,num_cpus);
    for(i=0; i<num_cpus; i++)  {
	ithread[i]=i;
	pthread_create(&tid[i], NULL, (void *)mafillvmt, (void *)&ithread[i]);
    }
    for(i=0; i<num_cpus; i++)  pthread_join(tid[i], NULL);
    
    SFREE(ithread);

    /* copying and accumulating the stiffnes and/or mass matrix */

#pragma omp parallel \
    default(none) \
    shared(neq,adv,adv1,num_cpus,nzs,auv,auv1,bv,bv1)	\
    private(i,j)
    {

	#pragma omp for
	for(i=0;i<*neq;i++){
	    adv[i]=adv1[i];
	    for(j=1;j<num_cpus;j++){
		adv[i]+=adv1[i+j**neq];
	    }
	}
	
	#pragma omp for
	for(i=0;i<2**nzs;i++){
	    auv[i]=auv1[i];
	    for(j=1;j<num_cpus;j++){
		auv[i]+=auv1[i+(long long)j*2**nzs];
	    }
	}
	
	#pragma omp for
	for(i=0;i<3**neq;i++){
	    bv[i]=bv1[i];
	    for(j=1;j<num_cpus;j++){
		bv[i]+=bv1[i+j*3**neq];
	    }
	}
    }

    SFREE(adv1);
    SFREE(auv1);
    SFREE(bv1);
  
  return;

}

/* subroutine for multithreading of mafillv */

void *mafillvmt(ITG *i){

    ITG indexadv,indexbv,nefa,nefb,nefdelta;
    long long indexauv;

    indexadv=*i**neq1;
    indexauv=(long long)*i*2**nzs1;
    indexbv=*i*3**neq1;
    
// ceil -> floor

    nefdelta=(ITG)floor(*nef1/(double)num_cpus);
    nefa=*i*nefdelta+1;
    nefb=(*i+1)*nefdelta;
// next line! -> all parallel sections
    if((*i==num_cpus-1)&&(nefb<*nef1)) nefb=*nef1;

    FORTRAN(mafillv,(nef1,ipnei1,neifa1,neiel1,vfa1,xxn1,area1,
	    &auv1[indexauv],&adv1[indexadv],jq1,irow1,nzs1,&bv1[indexbv],
            vel1,cosa1,umfa1,xlet1,xle1,gradvfa1,xxi1,
	    body1,volume1,ielfa1,lakonf1,ifabou1,nbody1,neq1,
	    dtimef1,velo1,veloo1,sel1,xrlfa1,gamma1,xxj1,nactdohinv1,a11,
	    a21,a31,flux1,&nefa,&nefb));

    return NULL;
}
