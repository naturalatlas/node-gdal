.PHONY: clean clean-test build rebuild release test test-concurrent authors

MOCHA_ARGS=-n expose-gc -R list --require ./test/_common.js test

all: build

authors:
	git shortlog -se \
	  | perl -spe 's/^\s+\d+\s+//' \
	  | sed -e '/^BrandonReavis/d' \
	  | sed -e '/brianreavis/d' \
	  | sed -e '/^brandonreavis/d' \
	  > AUTHORS
	echo "Zac McCormick <zac.mccormick@gmail.com>" >> AUTHORS

clean: clean-test
	@rm -rf ./build
	@rm -rf lib/binding

clean-test:
	@rm -f ./test/**/*.aux.xml
	@rm -rf ./test/**/*.tmp*
	@rm -rf ./test/data/**/*.tmp*

./node_modules/.bin/node-pre-gyp:
	npm install @mapbox/node-pre-gyp

build: ./node_modules/.bin/node-pre-gyp
	./node_modules/.bin/node-pre-gyp configure --enable-logging=true
	./node_modules/.bin/node-pre-gyp build

build-shared: ./node_modules/.bin/node-pre-gyp
	./node_modules/.bin/node-pre-gyp configure --enable-logging=true --shared_gdal=true
	./node_modules/.bin/node-pre-gyp build

rebuild:
	@make clean
	@make

test: clean-test build
	./node_modules/.bin/mocha $(MOCHA_ARGS)
	@make clean-test

test-shared: clean-test build-shared
	./node_modules/.bin/mocha $(MOCHA_ARGS)
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
	sed -i.bak 's/"version": "[^"]*"/"version": "$(version)"/' package-lock.json
	sed -i.bak 's/"version": "[^"]*"/"version": "$(version)"/' yuidoc.json
	rm *.bak
	git add .
	git commit --allow-empty -a -m "$(version)"
	git tag v$(version)
	git push origin master
	git push origin --tags
	@echo "\033[32mv${version} tagged (will publish to npm automatically when builds complete)\033[0;39m"
endif
