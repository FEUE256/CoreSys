# Makefile (project root)

APP_DIR = ../../src/db
MODE = Debug

install:
	cd $(APP_DIR) && npm install 

start:
	cd $(APP_DIR) && npm start

dev:
	cd $(APP_DIR) && npx electron .

clean:
	cd $(APP_DIR) && rm -rf node_modules dist package-lock.json ../../bin/$(MODE)/db/db.exe

reinstall: clean install

build:
	cd $(APP_DIR) && npm run dist

win:
	cd $(APP_DIR) && npx electron-builder --win

linux:
	cd $(APP_DIR) && npx electron-builder --linux

cp: 
	mv "$(APP_DIR)/dist/win-unpacked/DB App.exe" $(APP_DIR)/dist/win-unpacked/db.exe
	cp "$(APP_DIR)/dist/win-unpacked/db.exe" ../../bin/$(MODE)/db/db.exe
	cp ../../bin/$(MODE)/db/db.exe ../../dist/db.exe

all: install build cp

