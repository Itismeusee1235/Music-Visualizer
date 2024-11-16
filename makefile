EXEC = test 

$(EXEC): $(EXEC).cpp
	g++ $^ -o ./ObjectFile/$@  -lportaudio


