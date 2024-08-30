file:
	@gcc algebropiler.c fileParser.c -o algebropiler.out
input_text: file
	@./algebropiler.out sales.csv output.c
input_file: file
	@./algebropiler.out newFile.agc sales.csv output.c
output: 
	@gcc output.c -o test.out
run: output
	@./test.out
run_full: input_file run
	
clean:
	@rm *.out