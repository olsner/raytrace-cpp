# fast math reduced runtime from 15s to 12s, so seems useful :)
CXXFLAGS = -std=c++17 -O3 -g -march=native -ffast-math
CXXFLAGS += -MD -MP
LIBS += -ltbb

RAYTRACE_OBJS = raytrace.o scene.o
INTERSECT_OBJS = intersect.o

OBJS = $(RAYTRACE_OBJS) $(INTERSECT_OBJS)
DEPS = $(OBJS:.o=.d)

all: raytrace intersect

clean:
	rm -f raytrace $(OBJS) $(DEPS)

raytrace: $(RAYTRACE_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

intersect: $(INTERSECT_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

-include $(DEPS)
