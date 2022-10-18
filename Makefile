build-and-serve: build serve

build:
	emcc hello.cpp -o hello.html -s USE_SDL=2 -s USE_SDL_IMAGE=2 -lGLESv2 -lEGL -lm -lX11 -MJ a.o.json -gsource-map --source-map-base http://localhost:8000/

serve:
	python3 -m http.server

compile_commands: build
	echo "[" > compile_commands.json
	cat a.o.json >> compile_commands.json
	echo "]" >> compile_commands.json

clean:
	rm -f compile_commands.json\
	   *.o.json\
           *.html\
           *.wasm\
	   *.js

mac-tidy:
	/opt/homebrew/opt/llvm/bin/clang-tidy *.cpp

mac-format:
	/opt/homebrew/opt/llvm/bin/clang-format -style=llvm *.cpp -i
