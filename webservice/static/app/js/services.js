'use strict';

/* Services */

var cpuVisorServices = angular.module('cpuVisorServices', ['ngResource']);

cpuVisorServices.factory('StartQuery', ['$resource',
  function ($resource) {
    return $resource('/api/query/start_query',
      {},
      {
        get: {method: 'POST', params: {}}
      });
  }]);

cpuVisorServices.factory('AddTrs', ['$resource',
  function ($resource) {
    return $resource('/api/query/:qid/add_trs/:q',
      {},
      {
        get: {method: 'POST', params: {
          query_id: '@qid',
          query_str: '@q'
        }}
      });
  }]);

cpuVisorServices.factory('TrainModel', ['$resource',
  function ($resource) {
    return $resource('/api/query/:qid/train',
      {},
      {
        get: {method: 'POST', params: {
          query_id: '@qid'
        }}
      });
  }]);

cpuVisorServices.factory('Rank', ['$resource',
  function ($resource) {
    return $resource('/api/query/:qid/rank',
      {},
      {
        get: {method: 'POST', params: {
          query_id: '@qid'
        }}
      });
  }]);

cpuVisorServices.factory('Ranking', ['$resource',
  function ($resource) {
    return $resource('/api/query/:qid/ranking',
      {},
      {
        get: {method: 'GET', params: {
          query_id: '@qid'
        }}
      });
  }]);

cpuVisorServices.factory('FreeQuery', ['$resource',
  function ($resource) {
    return $resource('/api/query/:qid/free',
      {},
      {
        get: {method: 'GET', params: {
          query_id: '@qid'
        }}
      });
  }]);

cpuVisorServices.factory('NotificationSocket', function (socketFactory) {
  return socketFactory({
    ioSocket: io.connect('/api/notifications')
  });
});