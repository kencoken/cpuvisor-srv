'use strict';

/* Controllers */

var cpuVisorControllers = angular.module('cpuVisorControllers', ['cpuVisorDirectives']);

cpuVisorControllers.controller('LandingCtrl', ['$scope',
  function ($scope) {

    $scope.query_text = '';

    $scope.search = function(query_text) {
      if (query_text) {
        alert('Query text is: ' + query_text);
      }
    };

  }]);

cpuVisorControllers.controller('RankingCtrl', ['$scope', '$routeParams', '$location', '$http', 'Ranking', 'RankingUpTo', 'DsetClass', 'ClassPrecs',
  function ($scope, $routeParams, $location, $http, Ranking, RankingUpTo, DsetClass, ClassPrecs) {

    $scope.method_id = $routeParams.methodId;
    $scope.class_id = $routeParams.classId;
    $scope.class_name = '';
    $scope.pages_retrieved = 0;

    $scope.updating_prec_stat = 1;
    $scope.prec_stat_sml = 0.0;
    $scope.prec_stat = 0.0;
    $scope.prec_distr_ono_stat = 0.0;
    $scope.num_im_stat = 0;

    $scope.hide_fns = false;
    $scope.show_ono_distractors = false;

    DsetClass.get({class_id: $routeParams.classId}, function(dsetClassObj) {
      $scope.class_name = dsetClassObj.name;
    });

    Ranking.get({method_id: $routeParams.methodId, class_id: $routeParams.classId, page:0}, function(rankingObj) {
      $scope.ranking = rankingObj.rlist;
      $scope.pages_retrieved = 1;
    });

    ClassPrecs.get({method_id: $routeParams.methodId, class_id: $routeParams.classId}, function(classPrecsObj) {
      $scope.prec_stat_sml = classPrecsObj.prec[49];
      $scope.prec_stat = classPrecsObj.prec[99];
      $scope.num_im_stat = classPrecsObj.visited;
      $scope.prec_distr_ono_stat = classPrecsObj.prec_distr_ono[99];
      $scope.updating_prec_stat = Math.min(0, $scope.updating_prec_stat - 1);
    });

    $scope.loadMore = function() {
      if ($scope.pages_retrieved >= 1) {
        Ranking.get({method_id: $routeParams.methodId, class_id: $routeParams.classId, page:$scope.pages_retrieved}, function(rankingObj) {
          $scope.ranking = $scope.ranking.concat(rankingObj.rlist);
          $scope.pages_retrieved = $scope.pages_retrieved + 1;
        });
      }
    };

    $scope.willRefreshPrecStat = function() {
      $scope.updating_prec_stat = $scope.updating_prec_stat + 1;
    };

    $scope.failRefreshPrecStat = function() {
      $scope.updating_prec_stat = Math.min(0, $scope.updating_prec_stat - 1);
    };

    $scope.refreshPrecStat = function() {
      ClassPrecs.get({method_id: $routeParams.methodId, class_id: $routeParams.classId}, function(classPrecsObj) {
        $scope.prec_stat_sml = classPrecsObj.prec[49];
        $scope.prec_stat = classPrecsObj.prec[99];
        $scope.num_im_stat = classPrecsObj.visited;
        $scope.updating_prec_stat = Math.min(0, $scope.updating_prec_stat - 1);

        $scope.$apply();
      });
    };

    $scope.toggleShowFns = function() {
      $scope.hide_fns = !$scope.hide_fns;
    };

    $scope.showUpToN = function () {
      RankingUpTo.get({method_id: $routeParams.methodId, class_id: $routeParams.classId, upto:$scope.num_im_stat}, function(rankingObj) {
        $scope.ranking = rankingObj.rlist;
        $scope.pages_retrieved = rankingObj.page_count;

        //alert('retrieved ' + rankingObj.page_count + ' pages');
      });
    };

    $scope.toggleShowOnoDistractors = function() {
      $scope.show_ono_distractors = !$scope.show_ono_distractors;
    };

    $scope.tailDistractorsToFn = function() {

      function updateFnStatus(i) {
        var ritem = $scope.ranking[i];

        if (ritem.fn == 1) return false;
        if (ritem.is_distractor) {

          $scope.willRefreshPrecStat();
          ritem.fn = 1;

          $http({
            url: '/api/fns/' + $scope.class_id,
            method: 'POST',
            data: {value: 1,
              path: ritem.path},
            headers: {'Content-Type': 'application/json'}
          }).success(function (data, status, headers, config) {
            $scope.refreshPrecStat();
          }).error(function (data, status, headers, config) {
            ritem.fn = 0;
            $scope.failRefreshPrecStat();
          });
        }
        return true;
      }

      for (var i = $scope.ranking.length-1; i >= 0; --i) {
        if (!updateFnStatus(i)) break;
      }
    };

    $scope.goBack = function() {
      $location.path('/methods/' + $scope.method_id);
    };

  }]);
