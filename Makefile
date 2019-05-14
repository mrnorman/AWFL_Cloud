
KOKKOS_PATH = /home/imn/kokkos
KOKKOS_SRC_PATH = ${KOKKOS_PATH}
SRC = driver.cpp
EXE = cloudFV

default: main

CXX = mpic++
# CXX = ${KOKKOS_PATH}/bin/nvcc_wrapper -x cu -ccbin /usr/bin/mpic++
LINK = ${CXX}
# CFLAGS = -DARRAY_DEBUG -O1 -g -I${PNETCDF_PATH}/include
CXXFLAGS = -O3 -I${PNETCDF_PATH}/include
LDFLAGS = -L${PNETCDF_PATH}/lib -lpnetcdf

KOKKOS_DEVICES = "Serial"

# KOKKOS_DEVICES = "Cuda"
# KOKKOS_ARCH = "Kepler35"
# KOKKOS_CUDA_OPTIONS += "enable_lambda"

DEPFLAGS = -M
OBJ = $(notdir $(SRC:.cpp=.o))
LIB =

include ${KOKKOS_PATH}/Makefile.kokkos

main: $(EXE)
$(EXE): $(OBJ) $(KOKKOS_LINK_DEPENDS)
	$(LINK) $(KOKKOS_LDFLAGS) $(EXTRA_PATH) $(OBJ) $(KOKKOS_LIBS) $(LIB) -o $(EXE) $(LDFLAGS)

%.o:%.cpp $(KOKKOS_CPP_DEPENDS)
	$(CXX) $(KOKKOS_CPPFLAGS) $(KOKKOS_CXXFLAGS) $(CXXFLAGS) $(EXTRA_INC) -c $< -o $(notdir $@)

clean:
	rm -f *.gch *.o *.dat cloudFV

realclean: clean
	rm -f KokkosCore_config.h KokkosCore_config.tmp libkokkos.a

distclean: clean
	rm -f KokkosCore_config.h KokkosCore_config.tmp libkokkos.a


