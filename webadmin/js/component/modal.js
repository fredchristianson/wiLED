import {ComponentBase} from '../../drjs/browser/component.js';
import DOMEvent from '../../drjs/browser/dom-event.js';
import DOM from '../../drjs/browser/dom.js'
import HTMLTemplate from '../../drjs/browser/html-template.js'

const modalHTML= `
<div class="modal _drjsmodal">
    <div class="background">
    </div>
    <div class="content">
    </div>
</div>
`;

export class Modal {
    constructor(selector) {
        if ( selector instanceof HTMLTemplate) {
            this.contentTemplate = selector;
        } else {
            this.contentTemplate = new HTMLTemplate(DOM.first(selector));
        }
        this.modal = DOM.first('_drjsmodal');
        if (this.modal == null) {
            this.modalTemplate = new HTMLTemplate(modalHTML);
            this.modal = this.modalTemplate.getNodes()[0];
            document.body.appendChild(this.modal);
        }
        this.content = DOM.first(this.modal,'.content');
        this.content.innerHTML = '';
        this.contentTemplate.getNodes().forEach((node)=>{ this.content.appendChild(node);});


        // this.modal = DOM.createElement('div',{"@class":"modal"});
        // this.background = DOM.createElement('div',{"@class":"background"});
        // this.content =  DOM.createElement('div',{"@class":"content"});
        // this.modal.appendChild(this.content);
        // this.modal.appendChild(this.background);
        // document.body.appendChild(this.modal);
    }

    getDOM() { return this.content;}

    async show() {
        DOM.display(this.modal,"block");
        var modal = this.modal;
        this.promise = new Promise((resolve,reject) => {
            var clickListener = null;
            var keyListener = DOMEvent.listen('keyup',"*",(key,target,event)=>{
                if (key == 'Escape') {
                    DOMEvent.removeListener(clickListener);
                    DOMEvent.removeListener(keyListener);
                    DOM.remove(modal);
                    resolve();
                }
            });
            function onClick(button) {
                var response = DOM.getData(button,"value");
                var cancel = DOM.getData(button,"cancel");
                DOMEvent.removeListener(clickListener);
                DOMEvent.removeListener(keyListener);
                DOM.hide(modal);
                resolve(response); // always resolve to make it easier for caller (no try/catch)
                // if (response && !cancel) {
                //     resolve(response);
                // } else {
                //     reject(response);
                // }

            }
            clickListener = DOMEvent.listen("click",'.modal button',onClick);

        });
        return this.promise;
    }


}

export default Modal;