import assert from '../assert.js';
import Logger from '../logger.js';
import util from '../util.js';

const log = Logger.create("Page");

export class Page {
    constructor() {
        this.defaultPage = "index";
    }

    setDefaultPage(name) {
        this.defaultPage = name;
    }
    
    name() {
        var page = window.location.pathname.substr(1);
        if (util.isEmpty(page)){
            page = this.defaultPage;
        }
        return page;
    }

    set(name) {
        const newUrl = `/${name}`;
        window.history.pushState({},null,newUrl);
    }

    isLoggedIn() {
        return !util.isNull(this.user);
    }

    user() {
        return null;
    }
}

export default new Page();