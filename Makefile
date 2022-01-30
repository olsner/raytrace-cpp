CXXFLAGS = -std=c++17 -O3 -g -march=native
CXXFLAGS += -MD -MP
LIBS += -ltbb

RAYTRACE_OBJS = raytrace.o scene.o

OBJS = $(RAYTRACE_OBJS)
DEPS = $(OBJS:.o=.d)

all: raytrace

clean:
	rm -f raytrace $(OBJS) $(DEPS)

raytrace: $(RAYTRACE_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

-include $(DEPS)
