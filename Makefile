# Makefile for Pilot tutorial C and Fortran labs

# HPCVL global desktop; change to your local Pilot installation
#PILOTHOME = /work/whuettin/pilot-1.2a
PILOTHOME = /home/wolfgang/work/programming/pilot/pilot-1.2a

CC = mpicc 
#CPPFLAGS = -I$(PILOTHOME)/include
#LDFLAGS = -L$(PILOTHOME)/lib -lpilot
CPPFLAGS = -I$(PILOTHOME)/include
LDFLAGS = -L$(PILOTHOME)/lib


#FC = mpif90 -intel
#FFLAGS = -fpp -I$(PILOTHOME)/include
# LDFLAGS same as above


# "make labN" will compile labN.c using implicit make rules
#lab%: lab%.o
#	$(CC) $< $(LDFLAGS) -o $@


# "make flabN" will compile flabN.f90 and link flabN
#flab%.o: flab%.f90
#	$(FC) -c $< $(FFLAGS) -o $@

#flab%: flab%.o
#	$(FC) $< $(LDFLAGS) -o $@
#	rm $<

# modyfied to compile palin.c
palin: palin.o
	$(CC) $< $(LDFLAGS) -o $@

# modyfied to compile gameoflife.c
game: gameoflife.o
	$(CC) $< $(LDFLAGS) -o $@

clean:
	$(RM) lab? flab? *lab*.o* *.mod *palin*.o *gameoflife*.o
