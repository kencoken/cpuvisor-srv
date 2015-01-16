'use strict';

/* Services */

var cpuVisorServices = angular.module('cpuVisorServices', ['ngResource']);

cpuVisorServices.factory('RankingImageSharedState', function() {
  'use strict';

  var recently_expanded_cb = null;
  var recently_expanded = false;

  return {
    recently_expanded_cb: recently_expanded_cb,
    recently_expanded: recently_expanded
  };
});

cpuVisorServices.factory('StartQuery', ['$resource',
  function ($resource) {
    return $resource('/api/query/start_query',
      {},
      {
        execute: {method: 'POST'}
      });
  }]);

cpuVisorServices.factory('AddTrs', ['$resource',
  function ($resource) {
    return $resource('/api/query/:qid/add_trs/:q',
      {
        qid: '@qid',
        q: '@q'
      },
      {
        execute: {method: 'PUT'}
      });
  }]);

cpuVisorServices.factory('TrainModel', ['$resource',
  function ($resource) {
    return $resource('/api/query/:qid/train',
      {
        qid: '@qid'
      },
      {
        execute: {method: 'PUT'}
      });
  }]);

cpuVisorServices.factory('Rank', ['$resource',
  function ($resource) {
    return $resource('/api/query/:qid/rank',
      {
        qid: '@qid'
      },
      {
        execute: {method: 'PUT'}
      });
  }]);

cpuVisorServices.factory('Ranking', ['$resource',
  function ($resource) {
    return $resource('/api/query/:qid/ranking',
      {
        qid: '@qid'
      },
      {
        get: {method: 'GET'}
      });
  }]);

cpuVisorServices.factory('FreeQuery', ['$resource',
  function ($resource) {
    return $resource('/api/query/:qid/free',
      {
        qid: '@qid'
      },
      {
        execute: {method: 'PUT'}
      });
  }]);