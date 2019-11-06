#may not be useful anymore, bettre use CMake

CXX = g++
CXXFLAGS = -O3 -Wall -std=c++11
linkerFLAGS = -s -static-libstdc++ -static-libgcc -static -mwindows -mthreads

defines = -D__GNUWIN32__ -D__WXMSW__ -DwxUSE_UNICODE
wxLib = -lwxmsw30u_adv -lwxmsw30u_core -lwxbase30u -lwxpng -lwxzlib -lole32 -luuid -loleaut32 -lwinspool -lcomctl32

wxLibPath = C:\wxWidgets-3.0.4\lib\gcc_lib
wxIPath = C:\wxWidgets-3.0.4\include
wxIMSWU = C:\wxWidgets-3.0.4\lib\gcc_lib\mswu

obj = bin/obj/Main.o bin/obj/CeDImu.o \
bin/obj/MainFrame.o bin/obj/DisassemblerFrame.o bin/obj/GamePanel.o bin/obj/RAMSearchFrame.o bin/obj/RAMSearchList.o \
bin/obj/SCC68070.o bin/obj/Interpreter.o bin/obj/ConditionalTests.o bin/obj/Disassembler.o bin/obj/InstructionSet.o bin/obj/AddressingModes.o bin/obj/MemoryAccess.o \
bin/obj/SCC66470.o bin/obj/SCC66470DRAMInterface.o \
bin/obj/MCD212.o bin/obj/MCD212Registers.o bin/obj/MCD212DRAMInterface.o \
bin/obj/CDI.o bin/obj/CDIDirectory.o bin/obj/CDIFile.o bin/obj/Export.o

CeDImu :
	if not exist bin\obj\ mkdir bin\obj
	windres.exe -I$(wxIPath) -I$(wxIMSWU) -J rc -O coff -i Ressources/ressource.rc -o bin/obj/ressource.res
	$(CXX) $(CXXFLAGS) -I$(wxIPath) -I$(wxIMSWU) $(defines) -c src/CDI/CDI.cpp -o bin/obj/CDI.o
	$(CXX) $(CXXFLAGS) -I$(wxIPath) -I$(wxIMSWU) $(defines) -c src/CDI/CDIDirectory.cpp -o bin/obj/CDIDirectory.o
	$(CXX) $(CXXFLAGS) -I$(wxIPath) -I$(wxIMSWU) $(defines) -c src/CDI/CDIFile.cpp -o bin/obj/CDIFile.o
	$(CXX) $(CXXFLAGS) -I$(wxIPath) -I$(wxIMSWU) $(defines) -c src/CDI/Export.cpp -o bin/obj/Export.o
	$(CXX) $(CXXFLAGS) -I$(wxIPath) -I$(wxIMSWU) $(defines) -c src/Cores/SCC68070/SCC68070.cpp -o bin/obj/SCC68070.o
	$(CXX) $(CXXFLAGS) -I$(wxIPath) -I$(wxIMSWU) $(defines) -c src/Cores/SCC68070/Interpreter.cpp -o bin/obj/Interpreter.o
	$(CXX) $(CXXFLAGS) -I$(wxIPath) -I$(wxIMSWU) $(defines) -c src/Cores/SCC68070/MemoryAccess.cpp -o bin/obj/MemoryAccess.o
	$(CXX) $(CXXFLAGS) -I$(wxIPath) -I$(wxIMSWU) $(defines) -c src/Cores/SCC68070/Disassembler.cpp -o bin/obj/Disassembler.o
	$(CXX) $(CXXFLAGS) -I$(wxIPath) -I$(wxIMSWU) $(defines) -c src/Cores/SCC68070/InstructionSet.cpp -o bin/obj/InstructionSet.o
	$(CXX) $(CXXFLAGS) -I$(wxIPath) -I$(wxIMSWU) $(defines) -c src/Cores/SCC68070/AddressingModes.cpp -o bin/obj/AddressingModes.o
	$(CXX) $(CXXFLAGS) -I$(wxIPath) -I$(wxIMSWU) $(defines) -c src/Cores/SCC68070/ConditionalTests.cpp -o bin/obj/ConditionalTests.o
	$(CXX) $(CXXFLAGS) -I$(wxIPath) -I$(wxIMSWU) $(defines) -c src/Cores/SCC66470/DRAMInterface.cpp -o bin/obj/SCC66470DRAMInterface.o
	$(CXX) $(CXXFLAGS) -I$(wxIPath) -I$(wxIMSWU) $(defines) -c src/Cores/SCC66470/SCC66470.cpp -o bin/obj/SCC66470.o
	$(CXX) $(CXXFLAGS) -I$(wxIPath) -I$(wxIMSWU) $(defines) -c src/Cores/MCD212/DRAMInterface.cpp -o bin/obj/MCD212DRAMInterface.o
	$(CXX) $(CXXFLAGS) -I$(wxIPath) -I$(wxIMSWU) $(defines) -c src/Cores/MCD212/MCD212.cpp -o bin/obj/MCD212.o
	$(CXX) $(CXXFLAGS) -I$(wxIPath) -I$(wxIMSWU) $(defines) -c src/Cores/MCD212/Registers.cpp -o bin/obj/MCD212Registers.o
	$(CXX) $(CXXFLAGS) -I$(wxIPath) -I$(wxIMSWU) $(defines) -c src/GUI/MainFrame.cpp -o bin/obj/MainFrame.o
	$(CXX) $(CXXFLAGS) -I$(wxIPath) -I$(wxIMSWU) $(defines) -c src/GUI/GamePanel.cpp -o bin/obj/GamePanel.o
	$(CXX) $(CXXFLAGS) -I$(wxIPath) -I$(wxIMSWU) $(defines) -c src/GUI/RAMSearchList.cpp -o bin/obj/RAMSearchList.o
	$(CXX) $(CXXFLAGS) -I$(wxIPath) -I$(wxIMSWU) $(defines) -c src/GUI/RAMSearchFrame.cpp -o bin/obj/RAMSearchFrame.o
	$(CXX) $(CXXFLAGS) -I$(wxIPath) -I$(wxIMSWU) $(defines) -c src/GUI/DisassemblerFrame.cpp -o bin/obj/DisassemblerFrame.o
	$(CXX) $(CXXFLAGS) -I$(wxIPath) -I$(wxIMSWU) $(defines) -c src/CeDImu.cpp -o bin/obj/CeDImu.o
	$(CXX) $(CXXFLAGS) -I$(wxIPath) -I$(wxIMSWU) $(defines) -c src/Main.cpp -o bin/obj/Main.o
	$(CXX) -L$(wxLibPath) -o bin/CeDImu.exe $(obj) bin/obj/ressource.res $(linkerFLAGS) $(wxLib)
