import assert from '../assert.js';
import Logger from '../logger.js';
import util from '../util.js';
import httpRequest from './http-request.js';
import env from '../env.js';

const log = Logger.create("Api");

export class ApiError extends Error {
    constructor(request,params,response) {
        super(`API request failed: ${request}`);
        this.request = request;
        this.params = params;
        this.response = response;
    }
}

export class Api {
    constructor(basePath='/api/v1') {
        this.basePath = basePath;
    }

    async get(request,params={}) {
        assert.notEmpty(request);
        const url = util.combinePath(this.basePath,request);
        if (params == null) {
            params = {};
        }

        params.timestamp = Date.now();
        const response = await httpRequest.get(url,params,'json');
        if (response.success) {
            return response.data;
        }
        log.error(`API request `,request, `failed" `,response);
        throw new ApiError(request,params,response);
    }
}

const api = new Api(env.BASE_URL); 
export default api;