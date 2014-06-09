.PHONY: clean clean-test build rebuild release test test-concurrent

NODE_PATH:=$(shell pwd)/lib

all: build

clean: clean-test
	@rm -rf ./build
	@rm -rf lib/binding

clean-test:
	@rm -f ./test/**/*.aux.xml
	@rm -f ./test/**/*.tmp*

format-code:
	astyle \
		--indent=force-tab=4 \
		--indent-namespaces \
		--add-brackets \
		--style=stroustrup \
		./src/*
	@rm -rf ./src/*.orig

./node_modules/.bin/node-pre-gyp:
	npm install node-pre-gyp

build: ./node_modules/.bin/node-pre-gyp
	./node_modules/.bin/node-pre-gyp build

rebuild:
	@make clean
	@make

test: clean-test
	npm test
	@make clean-test

test-concurrent: clean-test
	node ./node_modules/.bin/_mocha \
		& node ./node_modules/.bin/_mocha \
		& node ./node_modules/.bin/_mocha \
		& node ./node_modules/.bin/_mocha \
		& node ./node_modules/.bin/_mocha \
		& node ./node_modules/.bin/_mocha
	@make clean-test

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