'use strict';

/* App Module */

var cpuVisorApp = angular.module('cpuVisorApp', [
  'ngRoute',
  'btford.socket-io',

  'cpuVisorControllers',
  'cpuVisorServices',
  'cpuVisorDirectives'
]);

cpuVisorApp.config(['$routeProvider',
  function ($routeProvider) {
    $routeProvider.
      when('/', {
        templateUrl: 'partials/landing.html',
        controller: 'LandingCtrl'
      }).
      when('/results', {
        templateUrl: 'partials/ranking.html',
        controller: 'RankingCtrl'
      }).
      otherwise({
        redirectTo: '/'
      });
  }]);
