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

static char *lakon1,*matname1,*objectset1;

static ITG *kon1,*ipkon1,*ne1,*nelcon1,*nrhcon1,*nalcon1,*ielmat1,*ielorien1,
    *norien1,*ntmat1_,*ithermal1,*iprestr1,*iperturb1,*iout1,*nmethod1,
    *nplicon1,*nplkcon1,*npmat1_,*mi1,*ielas1,*icmd1,*ncmat1_,*nstate1_,
    *istep1,*iinc1,calcul_fn1,calcul_qa1,calcul_cauchy1,iener1,ikin1,
    *nal=NULL,num_cpus,mt1,*nk1,*ne01,*mortar1,*ielprop1,*ndesi1,*nodedesi1,
    *ndirdesi1,*nobject1,numobject1,*nactdof1,*neq1;
    
static double *co1,*v1,*stx1,*elcon1,*rhcon1,*alcon1,*alzero1,*orab1,*t01,*t11,
    *prestr1,*eme1,*fn1=NULL,*qa1=NULL,*vold1,*veold1,*dtime1,*time1,
    *ttime1,*plicon1,*plkcon1,*xstateini1,*xstiff1,*xstate1,*stiini1,
    *vini1,*ener1,*eei1,*enerini1,*springarea1,*reltime1,*thicke1,*emeini1,
    *prop1,*pslavsurf1,*pmastsurf1,*clearini1,*distmin1,*g01,*dgdx1,*dgdv1,
    *sti1,*dgdxtot1,*dfextminds1,*df1;
    

void objectivesmmain_se(double *co,ITG *nk,ITG *kon,ITG *ipkon,char *lakon,ITG *ne,
       double *v,double *stn,ITG *inum,double *stx,double *elcon,ITG *nelcon,
       double *rhcon,ITG *nrhcon,double *alcon,ITG *nalcon,double *alzero,
       ITG *ielmat,ITG *ielorien,ITG *norien,double *orab,ITG *ntmat_,
       double *t0,
       double *t1,ITG *ithermal,double *prestr,ITG *iprestr,char *filab,
       double *eme,double *emn,
       double *een,ITG *iperturb,double *f,double *fn,ITG *nactdof,ITG *iout,
       double *qa,double *vold,double *b,ITG *nodeboun,ITG *ndirboun,
       double *xboun,ITG *nboun,ITG *ipompc,ITG *nodempc,double *coefmpc,
       char *labmpc,ITG *nmpc,ITG *nmethod,double *cam,ITG *neq,double *veold,
       double *accold,double *bet,double *gam,double *dtime,double *time,
       double *ttime,double *plicon,ITG *nplicon,double *plkcon,
       ITG *nplkcon,double *xstateini,double *xstiff,double *xstate,ITG *npmat_,
       double *epn,char *matname,ITG *mi,ITG *ielas,ITG *icmd,ITG *ncmat_,
       ITG *nstate_,
       double *stiini,double *vini,ITG *ikboun,ITG *ilboun,double *ener,
       double *enern,double *emeini,double *xstaten,double *eei,double *enerini,
       double *cocon,ITG *ncocon,char *set,ITG *nset,ITG *istartset,
       ITG *iendset,
       ITG *ialset,ITG *nprint,char *prlab,char *prset,double *qfx,double *qfn,
       double *trab,
       ITG *inotr,ITG *ntrans,double *fmpc,ITG *nelemload,ITG *nload,
       ITG *ikmpc,ITG *ilmpc,
       ITG *istep,ITG *iinc,double *springarea,double *reltime, ITG *ne0,
       double *xforc, ITG *nforc, double *thicke,
       double *shcon,ITG *nshcon,char *sideload,double *xload,
       double *xloadold,ITG *icfd,ITG *inomat,double *pslavsurf,
       double *pmastsurf,ITG *mortar,ITG *islavact,double *cdn,
       ITG *islavnode,ITG *nslavnode,ITG *ntie,double *clearini,
       ITG *islavsurf,ITG *ielprop,double *prop,double *energyini,
       double *energy,double *distmin,ITG *ndesi,ITG *nodedesi,
       ITG *ndirdesi,ITG *nobject,char *objectset,double *g0,double *dgdx,
       double *dgdv,double *sti,double *dgdxtot, double *dfextminds,
       double *df){

    ITG intpointvarm,calcul_fn,calcul_f,calcul_qa,calcul_cauchy,iener,ikin,
        intpointvart,mt=mi[1]+1,i,j,m,numobject;
      
    /* variables for multithreading procedure */
    
    ITG sys_cpus,*ithread=NULL;
    char *env,*envloc,*envsys;
    
    num_cpus = 0;
    sys_cpus=0;
    
    /* copying and accumulating dfextminds 

    NNEW(dfextminds1,double,*ndesi**neq);
    for(i=0;*ndesi**neq;++i){dfextminds1[i]=dfextminds[i];} */
    
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

    envloc = getenv("CCX_NPROC_RESULTS");
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

    if(*ne<num_cpus) num_cpus=*ne;
    
    pthread_t tid[num_cpus];
    
    /* 1. nodewise storage of the primary variables
       2. determination which derived variables have to be calculated */

    FORTRAN(resultsini,(nk,v,ithermal,filab,iperturb,f,fn,
       nactdof,iout,qa,vold,b,nodeboun,ndirboun,
       xboun,nboun,ipompc,nodempc,coefmpc,labmpc,nmpc,nmethod,cam,neq,
       veold,accold,bet,gam,dtime,mi,vini,nprint,prlab,
       &intpointvarm,&calcul_fn,&calcul_f,&calcul_qa,&calcul_cauchy,&iener,
       &ikin,&intpointvart,xforc,nforc));   

   /* next statement allows for storing the displacements in each
      iteration: for debugging purposes */

    if((strcmp1(&filab[3],"I")==0)&&(*iout==0)){
        FORTRAN(frditeration,(co,nk,kon,ipkon,lakon,ne,v,
                ttime,ielmat,matname,mi,istep,iinc,ithermal));
    }

        /* calculating the objective function and the derivatives */

        NNEW(fn1,double,num_cpus*mt**nk);     
        NNEW(qa1,double,num_cpus*3);
        NNEW(nal,ITG,num_cpus);
	NNEW(g01,double,num_cpus);
	NNEW(dgdxtot1,double,num_cpus**nk*numobject);

        co1=co;kon1=kon;ipkon1=ipkon;lakon1=lakon;ne1=ne;v1=v;
        stx1=stx;elcon1=elcon;nelcon1=nelcon;rhcon1=rhcon;
        nrhcon1=nrhcon;alcon1=alcon;nalcon1=nalcon;alzero1=alzero;
        ielmat1=ielmat;ielorien1=ielorien;norien1=norien;orab1=orab;
        ntmat1_=ntmat_;t01=t0;t11=t1;ithermal1=ithermal;prestr1=prestr;
        iprestr1=iprestr;eme1=eme;iperturb1=iperturb;iout1=iout;
        vold1=vold;nmethod1=nmethod;veold1=veold;dtime1=dtime;
        time1=time;ttime1=ttime;plicon1=plicon;nplicon1=nplicon;
        plkcon1=plkcon;nplkcon1=nplkcon;xstateini1=xstateini;
        xstiff1=xstiff;xstate1=xstate;npmat1_=npmat_;matname1=matname;
        mi1=mi;ielas1=ielas;icmd1=icmd;ncmat1_=ncmat_;nstate1_=nstate_;
        stiini1=stiini;vini1=vini;ener1=ener;eei1=eei;enerini1=enerini;
        istep1=istep;iinc1=iinc;springarea1=springarea;reltime1=reltime;
        calcul_fn1=calcul_fn;calcul_qa1=calcul_qa;calcul_cauchy1=calcul_cauchy;
        iener1=iener;ikin1=ikin;mt1=mt;nk1=nk;ne01=ne0;thicke1=thicke;
        emeini1=emeini;pslavsurf1=pslavsurf;clearini1=clearini;
        pmastsurf1=pmastsurf;mortar1=mortar;ielprop1=ielprop;prop1=prop;
        distmin1=distmin;ndesi1=ndesi;nodedesi1=nodedesi;
        ndirdesi1=ndirdesi;nobject1=nobject;objectset1=objectset;
        dgdx1=dgdx;dgdv1=dgdv;sti1=sti;df1=df;nactdof1=nactdof;neq1=neq;
        dfextminds1=dfextminds;
        
        if(((*nmethod!=4)&&(*nmethod!=5))||(iperturb[0]>1)){
                printf(" Using up to %" ITGFORMAT " cpu(s) for the stress calculation.\n\n", num_cpus);
        }
       
        NNEW(ithread,ITG,num_cpus);

    for(m=1;m<*nobject+1;m=m+1){
      if(strcmp1(&objectset[(m-1)*243],"MASS")==0){
    	 numobject=m;
    	 numobject1=numobject;
    	 /* OBJECTIVE: MASS */
    
    	 /* Total difference of the mass */
    	 /* create threads and wait */
    
    	 for(i=0; i<num_cpus; i++)  {
    	      ithread[i]=i;
    	      pthread_create(&tid[i], NULL, (void *)objectivemt_mass, (void *)&ithread[i]);
    	 }
    	  
    	 for(i=0; i<num_cpus; i++)  pthread_join(tid[i], NULL);
    	 }
    	  
      else if(strcmp1(&objectset[(m-1)*243],"SHAPEENERGY")==0){
    	 numobject=m;
    	 numobject1=numobject;
    	 /* OBJECTIVE: SHAPE ENERGY */
    	  
    	 /* Total difference of the internal shape energy */
    	 /* create threads and wait */
      
    	  for(i=0; i<num_cpus; i++)  {
    	      ithread[i]=i;
    	      pthread_create(&tid[i], NULL, (void *)objectivemt_shapeener, (void *)&ithread[i]);
    	  }
    	  
    	  for(i=0; i<num_cpus; i++)  pthread_join(tid[i], NULL);
    	  }
      }
        
      /* Passing of g0 */
      for(i=0;i<*nobject;i++){
          g0[i]=g01[i];
      }
      for(i=0;i<*nobject;i++){
          for(j=1;j<num_cpus;j++){
              g0[i]+=g01[i+j**nobject];
          }
      }
      SFREE(g01);

      /* Passing of dgdxtot */
      for(i=0;i<*ndesi**nobject;i++){
          dgdxtot[i]=dgdxtot1[i];
      }
      for(i=0;i<*ndesi**nobject;i++){
          for(j=1;j<num_cpus;j++){
              dgdxtot[i]+=dgdxtot1[i+j**ndesi**nobject];
          }
      }
      SFREE(dgdxtot1);

      SFREE(fn1);SFREE(ithread);SFREE(qa1);SFREE(nal); 
        
return;

}

/* ----------------------------------------------------------------*/
/* subroutine for multithreading: Differentiation of shape energy  */
/* ----------------------------------------------------------------*/

void *objectivemt_shapeener(ITG *i){

    ITG indexfn,indexqa,indexnal,nea,neb,nedelta;

    indexfn=*i*mt1**nk1;
    indexqa=*i*3;
    indexnal=*i;
    
// ceil -> floor

    nedelta=(ITG)floor(*ne1/(double)num_cpus);
    nea=*i*nedelta+1;
    neb=(*i+1)*nedelta;
// next line! -> all parallel sections
    if((*i==num_cpus-1)&&(neb<*ne1)) neb=*ne1;

    FORTRAN(objective_shapeener,(co1,kon1,ipkon1,lakon1,ne1,
          stx1,elcon1,nelcon1,rhcon1,nrhcon1,alcon1,nalcon1,alzero1,
          ielmat1,ielorien1,norien1,orab1,ntmat1_,t01,t11,ithermal1,prestr1,
          iprestr1,iperturb1,&fn1[indexfn],iout1,&qa1[indexqa],vold1,
          nmethod1,veold1,dtime1,time1,ttime1,plicon1,nplicon1,plkcon1,
          nplkcon1,xstateini1,xstiff1,xstate1,npmat1_,matname1,mi1,ielas1,
          icmd1,ncmat1_,nstate1_,stiini1,vini1,ener1,enerini1,istep1,iinc1,
          springarea1,reltime1,&calcul_qa1,&iener1,&ikin1,&nal[indexnal],          
          ne01,thicke1,emeini1,pslavsurf1,pmastsurf1,mortar1,clearini1,
          &nea,&neb,ielprop1,prop1,distmin1,ndesi1,nodedesi1,ndirdesi1,
          nobject1,g01,dgdx1,&numobject1,sti1,dgdxtot1,dfextminds1,df1,
          neq1,nactdof1,nk1));

    return NULL;
}

/* ---------------------------------------------------*/
/* subroutine for multithreading of objective_mass    */
/* ---------------------------------------------------*/

void *objectivemt_mass(ITG *i){

    ITG indexfn,indexqa,indexnal,nea,neb,nedelta;

    indexfn=*i*mt1**nk1;
    indexqa=*i*3;
    indexnal=*i;
    
// ceil -> floor

    nedelta=(ITG)floor(*ne1/(double)num_cpus);
    nea=*i*nedelta+1;
    neb=(*i+1)*nedelta;
// next line! -> all parallel sections
    if((*i==num_cpus-1)&&(neb<*ne1)) neb=*ne1;

    FORTRAN(objective_mass,(co1,kon1,ipkon1,lakon1,v1,nelcon1,rhcon1,          
          ielmat1,ielorien1,norien1,ntmat1_,vold1,matname1,mi1,&nal[indexnal],
          thicke1,mortar1,&nea,&neb,ielprop1,prop1,distmin1,ndesi1,nodedesi1,
          ndirdesi1,nobject1,g01,dgdxtot1,&numobject1));
          
    return NULL;
}