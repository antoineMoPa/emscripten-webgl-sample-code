build-and-serve: build serve

build:
	emcc hello.cpp -o hello.html -s USE_SDL=2 -s USE_SDL_IMAGE=2  -lGLESv2 -lEGL -lm -lX11

serve:
	python3 -m http.server
