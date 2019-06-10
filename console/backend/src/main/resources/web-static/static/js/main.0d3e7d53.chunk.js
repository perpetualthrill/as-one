(window.webpackJsonp=window.webpackJsonp||[]).push([[0],{149:function(e,t,n){e.exports=n(297)},154:function(e,t,n){},155:function(e,t,n){},296:function(e,t,n){},297:function(e,t,n){"use strict";n.r(t);var a=n(0),r=n.n(a),c=n(45),s=n.n(c),o=(n(154),n(155),n(29)),l=n.n(o),i=n(52),u=n(30),m=n(53),f=n.n(m),p={error:function(e){console.error(e)},log:function(e){console.log(e)}};function d(e,t){var n=Object(a.useRef)();Object(a.useEffect)(function(){n.current=e},[e]),Object(a.useEffect)(function(){if(null!==t){var e=setInterval(function(){n.current()},t);return function(){return clearInterval(e)}}},[t])}var h=n(54),v=n(16);function E(e){var t=Object(a.useState)([]),n=Object(u.a)(t,2),c=n[0],s=n[1],o=Object(a.useState)(!0),m=Object(u.a)(o,2),E=m[0],g=m[1],b=Object(a.useState)(null),w=Object(u.a)(b,2),y=w[0],O=w[1];function j(e,t){var n=1e3*e;return n+=t/1e6}function k(){return(k=Object(i.a)(l.a.mark(function t(){var n;return l.a.wrap(function(t){for(;;)switch(t.prev=t.next){case 0:return t.prev=0,t.next=3,f.a.get(e.url);case 3:n=t.sent,s(n.data),g(!1),t.next=11;break;case 8:t.prev=8,t.t0=t.catch(0),p.error(t.t0);case 11:case"end":return t.stop()}},t,null,[[0,8]])}))).apply(this,arguments)}Object(a.useEffect)(function(){var e={name:"readings",columns:["time","s1","s2","s3","s4"],points:[]};if(c.length>0){var t=c.reverse(),n=!0,a=!1,r=void 0;try{for(var s,o=t[Symbol.iterator]();!(n=(s=o.next()).done);n=!0){var l=s.value,i=[],u=j(l.timestamp.seconds,l.timestamp.nanos);i.push(u),i.push(l.s1),i.push(l.s2),i.push(l.s3),i.push(l.s4),e.points.push(i)}}catch(f){a=!0,r=f}finally{try{n||null==o.return||o.return()}finally{if(a)throw r}}var m=new v.TimeSeries(e);O(m)}},[c]),d(function(){!function(){k.apply(this,arguments)}()},250);return r.a.createElement(r.a.Fragment,null,E||null==y?"Loading ...":r.a.createElement(h.ChartContainer,{timeRange:y.timerange(),width:668},r.a.createElement(h.ChartRow,{height:"300",showGrid:!0},r.a.createElement(h.YAxis,{id:"axis1",min:300,max:650,width:28,type:"linear",format:".0f"}),r.a.createElement(h.Charts,null,r.a.createElement(h.LineChart,{axis:"axis1",series:y,columns:["s1","s2","s3","s4"],style:{s1:{stroke:"#a02c2c",opacity:.5},s2:{stroke:"#b03c3c",opacity:.5},s3:{stroke:"#c04c4c",opacity:.5},s4:{stroke:"#d05c5c",opacity:.5}}})))))}var g=n(61),b=n.n(g),w=n(23),y=n.n(w);function O(){var e="/sensors",t=Object(a.useState)([]),n=Object(u.a)(t,2),c=n[0],s=n[1];function o(){return(o=Object(i.a)(l.a.mark(function t(){var n;return l.a.wrap(function(t){for(;;)switch(t.prev=t.next){case 0:return t.prev=0,t.next=3,f.a.get(e);case 3:n=t.sent,s(n.data),t.next=10;break;case 7:t.prev=7,t.t0=t.catch(0),p.error(t.t0);case 10:case"end":return t.stop()}},t,null,[[0,7]])}))).apply(this,arguments)}return d(function(){!function(){o.apply(this,arguments)}()},1e3),Object(a.useEffect)(function(){}),c.map(function(e){return r.a.createElement(b.a,{key:e},r.a.createElement(y.a,{md:0,lg:1}),r.a.createElement(y.a,null,r.a.createElement(E,{url:"/sensors/"+e})),r.a.createElement(y.a,{md:0,lg:1}))})}var j=n(148),k=n.n(j),x=n(106),S=n.n(x);var C=function(){return r.a.createElement("div",{className:"App"},r.a.createElement(k.a,{fluid:"true"},r.a.createElement(b.a,null,r.a.createElement(y.a,{md:0,lg:1}),r.a.createElement(y.a,null,r.a.createElement(S.a,{expand:"lg",variant:"dark",bg:"dark ",style:{marginBottom:"40px"}},r.a.createElement(S.a.Brand,{href:"#"},"As One"))),r.a.createElement(y.a,{md:0,lg:1})),r.a.createElement(b.a,null,r.a.createElement(y.a,{md:0,lg:1}),r.a.createElement(y.a,null,r.a.createElement(O,null)),r.a.createElement(y.a,{md:0,lg:1}))))};Boolean("localhost"===window.location.hostname||"[::1]"===window.location.hostname||window.location.hostname.match(/^127(?:\.(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)){3}$/));n(296);s.a.render(r.a.createElement(C,null),document.getElementById("root")),"serviceWorker"in navigator&&navigator.serviceWorker.ready.then(function(e){e.unregister()})}},[[149,1,2]]]);
//# sourceMappingURL=main.0d3e7d53.chunk.js.map