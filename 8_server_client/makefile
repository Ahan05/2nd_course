run: server.c client.c clean
	touch *
	gcc client.c -pthread -lm  -o cl
	gcc server.c -pthread -lm  -o sr
	#./sr -p 4321 -a 0 -b 100000 -n 1
	#./cl -p 4321 -tn 1


clean:
	rm -rf sr cl
