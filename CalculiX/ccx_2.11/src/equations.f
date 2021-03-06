!
!     CalculiX - A 3-dimensional finite element program
!              Copyright (C) 1998-2015 Guido Dhondt
!
!     This program is free software; you can redistribute it and/or
!     modify it under the terms of the GNU General Public License as
!     published by the Free Software Foundation(version 2);
!     
!
!     This program is distributed in the hope that it will be useful,
!     but WITHOUT ANY WARRANTY; without even the implied warranty of 
!     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
!     GNU General Public License for more details.
!
!     You should have received a copy of the GNU General Public License
!     along with this program; if not, write to the Free Software
!     Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
!
      subroutine equations(inpc,textpart,ipompc,nodempc,coefmpc,
     &  nmpc,nmpc_,mpcfree,nk,co,trab,inotr,ntrans,ikmpc,ilmpc,
     &  labmpc,istep,istat,n,iline,ipol,inl,ipoinp,inp,ipoinpc,
     &  set,istartset,iendset,ialset,nset)
!
!     reading the input deck: *EQUATION
!
      implicit none
!
      character*1 inpc(*)
      character*20 labmpc(*)
      character*81 set(*),noset
      character*132 textpart(16)
!
      integer ipompc(*),nodempc(3,*),nmpc,nmpc_,mpcfree,istep,istat,
     &  n,i,j,ii,key,nterm,number,nk,inotr(2,*),ntrans,node,ndir,
     &  mpcfreeold,ikmpc(*),ilmpc(*),id,idof,itr,iline,ipol,inl,
     &  ipoinp(2,*),inp(3,*),ipoinpc(0:*),impcstart,impcend,i1,
     &  istartset(*),iendset(*),ialset(*),nset,k,l,m,index1,ipos,
     &  impc
!
      real*8 coefmpc(*),co(3,*),trab(7,*),a(3,3),x
!
      do m=2,n
         if(textpart(m)(1:9).eq.'REMOVEALL') then
            do j=1,nmpc
               index1=ipompc(j)
               if(index1.eq.0) cycle
               do
                  if(nodempc(3,index1).eq.0) then
                     nodempc(3,index1)=mpcfree
                     mpcfree=ipompc(j)
                     exit
                  endif
                  index1=nodempc(3,index1)
               enddo
               ipompc(j)=0
               ikmpc(j)=0
               ilmpc(j)=0
            enddo
            nmpc=0
            call getnewline(inpc,textpart,istat,n,key,iline,ipol,inl,
     &           ipoinp,inp,ipoinpc)
            return
         elseif(textpart(m)(1:6).eq.'REMOVE') then
            do
               call getnewline(inpc,textpart,istat,n,key,iline,ipol,inl,
     &              ipoinp,inp,ipoinpc)
               if((istat.lt.0).or.(key.eq.1)) return
!     
               read(textpart(2)(1:10),'(i10)',iostat=istat) impcstart
               if(istat.gt.0) call inputerror(inpc,ipoinpc,iline,
     &"*EQUATION%")
!     
               if(textpart(3)(1:1).eq.' ') then
                  impcend=impcstart
               else
                  read(textpart(3)(1:10),'(i10)',iostat=istat) impcend
                  if(istat.gt.0) call inputerror(inpc,ipoinpc,iline,
     &"*EQUATION%")
               endif
!     
               read(textpart(1)(1:10),'(i10)',iostat=istat) l
               if(istat.eq.0) then
                  if((l.gt.nk).or.(l.le.0)) then
                     write(*,*) '*ERROR reading *BOUNDARY:'
                     write(*,*) '       node ',l,' is not defined'
                     call exit(201)
                  endif
                  do i1=impcstart,impcend
                     idof=8*(l-1)+i1
                     call nident(ikmpc,idof,nmpc,id)
                     if(id.gt.0) then
                        if(ikmpc(id).eq.idof) then
                           impc=ilmpc(id)
                           call mpcrem(impc,mpcfree,nodempc,nmpc,
     &                          ikmpc,ilmpc,labmpc,coefmpc,ipompc)
                           cycle
                        endif
                     endif
                     write(*,*) 
     &                    '*WARNING reading *EQUATION: MPC to remove'
                     write(*,*) '         is not defined; node:',l
                     write(*,*) '         degree of freedom:',i1
                  enddo
               else
                  read(textpart(1)(1:80),'(a80)',iostat=istat) noset
                  noset(81:81)=' '
                  ipos=index(noset,' ')
                  noset(ipos:ipos)='N'
                  do i=1,nset
                     if(set(i).eq.noset) exit
                  enddo
                  if(i.gt.nset) then
                     noset(ipos:ipos)=' '
                     write(*,*) '*ERROR reading *BOUNDARY: node set ',
     &                       noset
                     write(*,*) '  has not yet been defined. '
                     call inputerror(inpc,ipoinpc,iline,
     &"*EQUATION%")
                     call exit(201)
                  endif
                  do j=istartset(i),iendset(i)
                     if(ialset(j).gt.0) then
                        k=ialset(j)
                        do i1=impcstart,impcend
                           idof=8*(k-1)+i1
                           call nident(ikmpc,idof,nmpc,id)
                           if(id.gt.0) then
                              if(ikmpc(id).eq.idof) then
                                 impc=ilmpc(id)
                                 call mpcrem(impc,mpcfree,nodempc,
     &                                nmpc,ikmpc,ilmpc,labmpc,coefmpc,
     &                                ipompc)
                                 cycle
                              endif
                           endif
                           write(*,*) 
     &                       '*WARNING reading *EQUATION: MPC to remove'
                           write(*,*) '         is not defined; node:',k
                           write(*,*) '         degree of freedom:',i1
                        enddo
                     else
                        k=ialset(j-2)
                        do
                           k=k-ialset(j)
                           if(k.ge.ialset(j-1)) exit
                           do i1=impcstart,impcend
                              idof=8*(k-1)+i1
                              call nident(ikmpc,idof,nmpc,id)
                              if(id.gt.0) then
                                 if(ikmpc(id).eq.idof) then
                                    impc=ilmpc(id)
                                    call mpcrem(impc,mpcfree,
     &                                 nodempc,nmpc,ikmpc,ilmpc,labmpc,
     &                                 coefmpc,ipompc)
                                    cycle
                                 endif
                              endif
                              write(*,*) 
     &                       '*WARNING reading *EQUATION: MPC to remove'
                              write(*,*) 
     &                            '         is not defined; node:',k
                              write(*,*)'         degree of freedom:',i1
                           enddo
                        enddo
                     endif
                  enddo
               endif
            enddo
            return
         else
             write(*,*) 
     &          '*WARNING reading *EQUATION: parameter not recognized:'
             write(*,*) '         ',
     &            textpart(m)(1:index(textpart(m),' ')-1)
             call inputwarning(inpc,ipoinpc,iline,
     &"*EQUATION%")
          endif
       enddo
!
      if(istep.gt.0) then
         write(*,*) 
     &       '*ERROR reading *EQUATION: *EQUATION should be placed'
         write(*,*) '  before all step definitions'
         call exit(201)
      endif
!
      do
         call getnewline(inpc,textpart,istat,n,key,iline,ipol,inl,
     &        ipoinp,inp,ipoinpc)
         if((istat.lt.0).or.(key.eq.1)) return
         read(textpart(1)(1:10),'(i10)',iostat=istat) nterm
!
         nmpc=nmpc+1
         if(nmpc.gt.nmpc_) then
            write(*,*) '*ERROR reading *EQUATION: increase nmpc_'
            call exit(201)
         endif
!
         labmpc(nmpc)='                    '
         ipompc(nmpc)=mpcfree
         ii=0
!
         do
            call getnewline(inpc,textpart,istat,n,key,iline,ipol,inl,
     &           ipoinp,inp,ipoinpc)
            if((istat.lt.0).or.(key.eq.1)) then
               write(*,*) '*ERROR reading *EQUATION: mpc definition ',
     &               nmpc
               write(*,*) '  is not complete. '
               call inputerror(inpc,ipoinpc,iline,
     &"*EQUATION%")
               call exit(201)
            endif
!
            do i=1,n/3
!
               read(textpart((i-1)*3+1)(1:10),'(i10)',iostat=istat) node
               if(istat.gt.0) call inputerror(inpc,ipoinpc,iline,
     &"*EQUATION%")
               if((node.gt.nk).or.(node.le.0)) then
                  write(*,*) '*ERROR reading *EQUATION:'
                  write(*,*) '       node ',node,' is not defined'
                  call exit(201)
               endif
!
               read(textpart((i-1)*3+2)(1:10),'(i10)',iostat=istat) ndir
               if(istat.gt.0) call inputerror(inpc,ipoinpc,iline,
     &"*EQUATION%")
               if(ndir.le.3) then
               elseif(ndir.eq.4) then
                  ndir=5
               elseif(ndir.eq.5) then
                  ndir=6
               elseif(ndir.eq.6) then
                  ndir=7
               elseif(ndir.eq.8) then
                  ndir=4
               elseif(ndir.eq.11) then
                  ndir=0
               else
                  write(*,*) '*ERROR reading *EQUATION:'
                  write(*,*) '       direction',ndir,' is not defined'
                  call exit(201)
               endif
!
               read(textpart((i-1)*3+3)(1:20),'(f20.0)',iostat=istat) x
               if(istat.gt.0) call inputerror(inpc,ipoinpc,iline,
     &"*EQUATION%")
!
!              check whether the node is transformed
!
               if(ntrans.le.0) then
                  itr=0
               elseif(inotr(1,node).eq.0) then
                  itr=0
               else
                  itr=inotr(1,node)
               endif
!
               if((itr.eq.0).or.(ndir.eq.0).or.(ndir.eq.4)) then
                  nodempc(1,mpcfree)=node
                  nodempc(2,mpcfree)=ndir
                  coefmpc(mpcfree)=x
!
!                 updating ikmpc and ilmpc
!
                  if(ii.eq.0) then
                     idof=8*(node-1)+ndir
                     call nident(ikmpc,idof,nmpc-1,id)
                     if(id.gt.0) then
                        if(ikmpc(id).eq.idof) then
                           write(*,100)
     &                   (ikmpc(id))/8+1,ikmpc(id)-8*((ikmpc(id))/8)
                           call exit(201)
                        endif
                     endif
                     do j=nmpc,id+2,-1
                        ikmpc(j)=ikmpc(j-1)
                        ilmpc(j)=ilmpc(j-1)
                     enddo
                     ikmpc(id+1)=idof
                     ilmpc(id+1)=nmpc
                  endif
!
                  mpcfreeold=mpcfree
                  mpcfree=nodempc(3,mpcfree)
                  if(mpcfree.eq.0) then
                     write(*,*) 
     &                  '*ERROR reading *EQUATION: increase nmpc_'
                     call exit(201)
                  endif
               else
                  call transformatrix(trab(1,inotr(1,node)),
     &                 co(1,node),a)
!
                  number=ndir-1
                  if(ii.eq.0) then
!
!                    determining which direction to use for the
!                    dependent side: should not occur on the dependent
!                    side in another MPC and should have a nonzero
!                    coefficient
!
                     do j=1,3
                        number=number+1
                        if(number.gt.3) number=1
                        idof=8*(node-1)+number
                        call nident(ikmpc,idof,nmpc-1,id)
                        if(id.gt.0) then
                           if(ikmpc(id).eq.idof) then
                              cycle
                           endif
                        endif
                        if(dabs(a(number,ndir)).lt.1.d-5) cycle
                        exit
                     enddo
                     if(j.gt.3) then
                        write(*,*) 
     &                    '*ERROR reading *EQUATION: SPC in node'
                        write(*,*) node,' in transformed coordinates'
                        write(*,*) ' cannot be converted in MPC: all'
                        write(*,*) ' DOFs in the node are used as'
                        write(*,*) ' dependent nodes in other MPCs'
                        call exit(201)
                     endif
                     number=number-1
!
!                    updating ikmpc and ilmpc
!
                     do j=nmpc,id+2,-1
                        ikmpc(j)=ikmpc(j-1)
                        ilmpc(j)=ilmpc(j-1)
                     enddo
                     ikmpc(id+1)=idof
                     ilmpc(id+1)=nmpc
                  endif
!
                  do j=1,3
                     number=number+1
                     if(number.gt.3) number=1
                     if(dabs(a(number,ndir)).lt.1.d-5) cycle
                     nodempc(1,mpcfree)=node
                     nodempc(2,mpcfree)=number
                     coefmpc(mpcfree)=x*a(number,ndir)
                     mpcfreeold=mpcfree
                     mpcfree=nodempc(3,mpcfree)
                     if(mpcfree.eq.0) then
                        write(*,*) 
     &                    '*ERROR reading *EQUATION: increase nmpc_'
                        call exit(201)
                     endif
                  enddo
               endif
!
               ii=ii+1
            enddo
!
            if(ii.eq.nterm) then
               nodempc(3,mpcfreeold)=0
               exit
            endif
         enddo
      enddo
!
 100  format(/,'*ERROR reading *EQUATION: the DOF corresponding to',
     &           /,'node ',i10,' in direction',i1,' is detected on',
     &           /,'the dependent side of two different MPC''s') 
      return
      end

