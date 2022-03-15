import {ComponentBase} from '../../drjs/browser/component.js';
import {app} from '../../drjs/browser/application.js';
import dom from '../../drjs/browser/dom.js';
import page from '../../drjs/browser/page.js';

export class HeaderComponent extends ComponentBase{
    constructor(selector, htmlName='header') {
        super(selector,htmlName);
    }

    attach(parent) {
        this.parent = parent;
        this.setupNavHandlers(parent);
    }

    setupNavHandlers(element) {
        const links = dom.find(element,'#main-nav a');
        const pageName = page.name();
        const loggedIn = page.isLoggedIn();
        links.forEach(link=>{
            const linkPage = link.getAttribute('href');
            const ifLoggedIn = link.dataset.ifLoggedIn;
            const ifNotLoggedIn = link.dataset.ifNotLoggedIn;
            if ((ifLoggedIn && !loggedIn) ||
                (ifNotLoggedIn && loggedIn)) {
                    dom.hide(link);
            } else {
                dom.show(link);
            }
            if (pageName == linkPage) {
                dom.addClass(link,'active');
            }
        });
    }
}

export default HeaderComponent;