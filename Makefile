.PHONY: clean clean-test build rebuild release test test-concurrent format-code authors docs

MOCHA_ARGS=test -R list -gc --require ./test/_common.js

all: build

authors:
	git shortlog -se \
	  | perl -spe 's/^\s+\d+\s+//' \
	  | sed -e '/^BrandonReavis/d' \
	  | sed -e '/^brandonreavis/d' \
	  > AUTHORS
	echo "Zac McCormick <zac.mccormick@gmail.com>" >> AUTHORS

./node_modules/gh-pages:
	npm install gh-pages@0.2.0

./node_modules/yuidoc-bootstrap-theme:
	npm install yuidoc-bootstrap-theme@1.0.4

./node_modules/.bin/yuidoc:
	npm install yuidocjs@0.3.50

docs: ./node_modules/.bin/yuidoc ./node_modules/yuidoc-bootstrap-theme
	./node_modules/.bin/yuidoc --extension .js,.cpp
	@echo "\033[32mDocumentation generated: ../node-gdal-yui-docs\033[0;39m"

publish-docs: docs ./node_modules/gh-pages
	node ./scripts/publish-docs.js

clean: clean-test
	@rm -rf ./build
	@rm -rf lib/binding

clean-test:
	@rm -f ./test/**/*.aux.xml
	@rm -rf ./test/**/*.tmp*
	@rm -rf ./test/data/**/*.tmp*

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
	./node_modules/.bin/node-pre-gyp build --enable-logging=true

build-shared: ./node_modules/.bin/node-pre-gyp
	./node_modules/.bin/node-pre-gyp build --enable-logging=true --shared_gdal=true

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
	rm -rf node_modules
	npm install
	make test
	@make authors
	sed -i.bak 's/"version": "[^"]*"/"version": "$(version)"/' package.json
	sed -i.bak 's/"version": "[^"]*"/"version": "$(version)"/' yuidoc.json
	rm *.bak
	git add .
	git commit -a -m "Released $(version). [publish binary]"
	git tag v$(version)
	git push origin master
	git push origin --tags
	npm publish
	@echo "\033[32mv${version} released\033[0;39m"
endif