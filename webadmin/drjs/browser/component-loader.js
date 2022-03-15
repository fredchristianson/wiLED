import assert from '../assert.js';
import Logger from '../logger.js';
import util from '../util.js';
import httpRequest from './http-request.js';

const log = Logger.create("ComponentLoader");

export class ComponentLoader {
    constructor() {
        this.componentDirectory = 'component';
    }

    async load(htmlFile) {
        assert.notEmpty(htmlFile);
        const fileExt = htmlFile.split('.');
        var fileName = htmlFile;
        var ext = '.html';
        if (fileExt.length > 1) {
            ext = fileExt[fileExt.length-2];
            file = htmlFile.substr(htmlFile.length-ext.length);
        }
        var nocache='?a='+Date.now();
        const filename=`${this.componentDirectory}/${fileName}${ext}${nocache}`;
        const contents = await httpRequest.get(filename);
        const htmlObject = document.createElement('div');
        htmlObject.innerHTML = contents.trim();
        const elements = Array.from(htmlObject.childNodes);
        return elements;
    }
}

const componentLoader = new ComponentLoader();
export default componentLoader;