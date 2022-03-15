import {ComponentBase} from '../../drjs/browser/component.js';
import assert from '../../drjs/assert.js';
import util from '../../drjs/util.js';
import dom from '../../drjs/browser/dom.js';
import Pager from './pager.js';

import {HtmlTemplate, HtmlValue,TextValue,AttributeValue,DataValue } from '../../drjs/browser/html-template.js';

export class ColorComponent extends ComponentBase{
    constructor(selector, htmlName='color') {
        super(selector,htmlName);
        this.pageSize = 10;
    }

    async onAttached(elements,parent){
        /*
        this.list = dom.first(parent,'.image-list');
        this.rowTemplate = new HtmlTemplate(dom.first('#photo-row'));
        assert.notNull(this.list,'unable to find .image-list element');
        this.itemContainer = dom.first(this.list,'.items');
        assert.notNull(this.itemContainer,'unable to find .items element');
        this.pager = new Pager(dom.first(parent,'.pager'),'pager');
        this.itemContainer.innerHTML = 'loading...';
        this.loadItems(1,this.pageSize);
        this.itemContainer.innerHTML = '';
        */
    }

    async loadItems(startItem,count) {
        const response = await api.get('localPhotos',{start:1,count:10});
        response.items.forEach(item=>{
            const values = {
                '.name': item.name,
                '.thumbnail': new AttributeValue('src',item.path),
                '.timestamp': util.formatTime(item.mtime,"mm/dd/yy HH:MM ampm")
            };
            dom.append(this.itemContainer,this.rowTemplate.fill(values));
        });
        this.pager.setTotalItemCount(response.page.totalItemCount);
    }
}

export default ColorComponent;