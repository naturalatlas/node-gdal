.PHONY: configure clean build release test

MOCHA=node_modules/.bin/mocha
NODE_PATH:=$(shell pwd)/lib

all: build

configure:
	node-gyp configure

build:
	node-gyp build

clean:
	node-gyp clean
	rm -rf build

test:
	$(MOCHA) -R list

release:
ifeq ($(strip $(version)),)
	@echo "\033[31mERROR:\033[0;39m No version provided."
	@echo "\033[1;30mmake release version=1.0.0\033[0;39m"
else
	sed -i.bak 's/"version": "[^"]*"/"version": "$(version)"/' package.json
	rm *.bak
	git add .
	git commit -a -m "Released $(version)."
	git tag v$(version)
	git push origin master
	git push origin --tags
	npm publish
	@echo "\033[32mv${version} released\033[0;39m"
endif