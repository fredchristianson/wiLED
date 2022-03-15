"user strict";
import {ComponentBase} from '../../drjs/browser/component.js';
import assert from '../../drjs/assert.js';
import util from '../../drjs/util.js';
import dom from '../../drjs/browser/dom.js';

export class PagerComponent extends ComponentBase{
    constructor(parent,htmlName='pager',pageSize=10,first=1,itemCount=0) {
        super(parent,htmlName);
        this.pageSize = pageSize;
        this.first = first;
        this.currentItem = -1;
        this.itemCount = itemCount;
    }

    setTotalItemCount(count) {
        this.totalItemCount = count;
        this.pageCount = this.totalItemCount/this.pageSize;
        this.updateButtons();
    }

    setPosition(itemNumber,totalItemCount=null) {
        this.currentItem = itemNumber;
        if (!util.isNull(totalItemCount)) {
            this.totalItemCount = totalItemCount;
        }
        this.updateButtons();
    }



    async onAttach(elements,parent){
        this.parent = parent;
        this.elements = elements;
        this.updateButtons();
    }

    updateButtons(){
        if (this.parent == null) {
            return;
        }
        const currentButtons = dom.find(this.parent,'li');
        var insertAfter = currentButtons[1];
        const pageCount = this.pageCount;
        const firstPageButton = Math.max(1,this.currentItem-5);
        const lastPageButton = Math.min(pageCount,this.currentItem+5);
        for(var idx=firstPageButton;idx<=lastPageButton;idx++) {
            const exists = dom.first(this.parent,`li[data-page-number="${idx}"]`);
            if (!exists) {
                const newButton = `<li data-page-number=${idx}>${idx}</li>`;
                insertAfter = dom.insertAfter(insertAfter,newButton);
            }
        }
        var next = insertAfter.nextElementSibling;
        while(!dom.hasClass(next,'standard-button')){
            const cur = next;
            next = cur.nextElementSibling;
            dom.remove(cur);
        }


    }
}

export default PagerComponent;