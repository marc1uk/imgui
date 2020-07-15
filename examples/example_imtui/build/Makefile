PKGS    := ncursesw
NCCFLAGS  := $(shell pkg-config --cflags $(PKGS))
NCLDFLAGS := $(shell pkg-config --libs $(PKGS))
# -lncurses -limtui-core -ltinfo -lncursesw? < 'w' adds support for wide chars

CXXFLAGS= -std=c++14

ADDITIONAL_LIBRARIES=
ifeq ($(UNAME_S), Darwin) #APPLE
	ADDITIONAL_LIBRARIES= -framework Cocoa
endif

all: imtui-example-ncurses0

imgui.cpp.o: ../../../imgui.cpp
	g++ $(CXXFLAGS) -fPIC -I../../.. $(ADDITIONAL_LIBRARIES) -MD -MP -c -o $@ $^

imgui_draw.cpp.o: ../../../imgui_draw.cpp
	g++ $(CXXFLAGS) -fPIC -I../../.. $(ADDITIONAL_LIBRARIES) -MD -MP -c -o $@ $^

imgui_demo.cpp.o: ../../../imgui_demo.cpp
	g++ $(CXXFLAGS) -fPIC -I../../.. $(ADDITIONAL_LIBRARIES) -MD -MP -c -o $@ $^

imgui_widgets.cpp.o: ../../../imgui_widgets.cpp
	g++ $(CXXFLAGS) -fPIC -I../../.. $(ADDITIONAL_LIBRARIES) -MD -MP -c -o $@ $^

libimgui.a: imgui.cpp.o imgui_draw.cpp.o imgui_demo.cpp.o imgui_widgets.cpp.o
	ar qc $@ $^
	ranlib libimgui.a

# core
imtui-impl-text.cpp.o: ../imtui-impl-text.cpp
	g++ $(CXXFLAGS) -fPIC -I../ -I../../ -I../include/ -MD -MP -c -o $@ $^

libimtui-core.a: imtui-impl-text.cpp.o
	ar qc $@ $^
	ranlib  $@

# ncurses
imtui-impl-ncurses.cpp.o: ../imtui-impl-ncurses.cpp
	g++ $(CXXFLAGS) -fPIC -I../.. -I../include/ $(NCLDFLAGS) $(NCCFLAGS) -MD -MP -c -o $@ $^

libimtui-ncurses.a: imtui-impl-ncurses.cpp.o
	ar qc $@ $^
	ranlib $@

# common
libimtui-examples-common.a: imtui-impl-ncurses.cpp.o
	ar qc $@ $^
	ranlib $@

# main
imtui-example-ncurses0: libimgui.a libimtui-core.a libimtui-ncurses.a libimtui-examples-common.a ../ncurses0/main.cpp
	g++ $(CXXFLAGS)  $^ -I../../ -I../include/ libimgui.a libimtui-core.a libimtui-ncurses.a -lcurses libimtui-examples-common.a -lform -ldl -MD -MP -o $@

.PHONY: clean

clean:
	rm *.a *.d *.o imtui-example-ncurses0

#note: libraries must be after the object that uses them, so put your sources ($^) at the front!
