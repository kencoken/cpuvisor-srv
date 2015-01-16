'use strict';

/* Directives */

var cpuVisorDirectives = angular.module('cpuVisorDirectives', []);

cpuVisorDirectives.directive('rankingImage', ['$http', 'RankingImageSharedState',
function ($http, RankingImageSharedState) {
    return {
      restrict: 'E',
      scope: {
        img_src: '@src',
        anno: '@anno',
        path: '@path',
        score: '@score'
      },
      templateUrl: 'templates/rankingImage.html',
      replace: true,
      link: function (scope, element, attrs) {

        scope.display_expanded_cb = null;
        scope.display_expanded = false;

        scope.processClick = function() {
          // nothing at the moment
        };

        scope.processMouseEnter = function() {
          if (RankingImageSharedState.recently_expanded_cb) {
            window.clearTimeout(RankingImageSharedState.recently_expanded_cb);
          }

          var timeout_for_expand = 2000;
          if (RankingImageSharedState.recently_expanded) {
            timeout_for_expand = 0;
          }

          scope.display_expanded_cb = window.setTimeout(function() {
            scope.display_expanded = true;
            RankingImageSharedState.recently_expanded = true;
            scope.$apply();
          }, timeout_for_expand);
        };

        scope.processMouseLeave = function() {
          if (scope.display_expanded_cb) {
            window.clearTimeout(scope.display_expanded_cb);
          }
          scope.display_expanded = false;

          if (RankingImageSharedState.recently_expanded_cb) {
            window.clearTimeout(RankingImageSharedState.recently_expanded_cb);
          }
          RankingImageSharedState.recently_expanded_cb = window.setTimeout(function() {
            RankingImageSharedState.recently_expanded = false;
            scope.$apply();
          }, 500);
        }

      }
    };
  }
]);
