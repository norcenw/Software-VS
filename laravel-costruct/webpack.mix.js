const mix = require("laravel-mix");

mix
  .js("src/js/app.js", "js/app.js")
  .js("src/js/3d.js", "js/3d.js")
  .js("src/js/dict.js", "js/dict.js")
  .js("src/js/ai.js", "js/ai.js")
  .sass("src/scss/style.scss", "css/style.css")
  .copy("src/**.html", "dist")
  .copyDirectory("src/elements", "dist/resources")
  .setPublicPath("dist")
  .version();