import {ComponentBase} from '../../drjs/browser/component.js';
import assert from '../../drjs/assert.js';
import util from '../../drjs/util.js';
import dom from '../../drjs/browser/dom.js';
import Pager from './pager.js';

import {HtmlTemplate, HtmlValue,TextValue,AttributeValue,DataValue } from '../../drjs/browser/html-template.js';

export class WhiteComponent extends ComponentBase{
    constructor(selector, htmlName='white') {
        super(selector,htmlName);
        this.pageSize = 10;
    }

    async onAttached(elements,parent){

    }


}

export default WhiteComponent;