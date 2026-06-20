APP_DIR := ../../src/db
MODE := Debug

ifeq ($(NO_NPM),1)
ALL_TARGET := nop
else
ALL_TARGET := install build cp
endif

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
	cp "$(APP_DIR)/dist/win-unpacked/DB App.exe" "$(APP_DIR)/dist/win-unpacked/db.exe"
	cp "$(APP_DIR)/dist/win-unpacked/db.exe" ../../bin/$(MODE)/db/db.exe
	cp ../../bin/$(MODE)/db/db.exe ../../dist/db.exe

nop:
	@echo "NO_NPM is set — skipping npm/build steps"

all: $(ALL_TARGET)