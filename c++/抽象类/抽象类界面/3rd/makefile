#Human:main.o Chinese.o Englishman.o Human.o
#	g++ -o $@ $^
Human:main.o libHuman.so
	g++ -o $@ $< -L./ -lHuman
%.o:%.cpp
	g++ -fPIC -c -o $@ $<
libHuman.so:Englishman.o Chinese.o Human.o
	g++ -shared  -o $@ $^
clean:
	rm -f *.o Human


#运行时需要指定库的路径
#LD_LIBRARY_PATH=./ ./Human
