module.exports = {
  resolve: {
    extensions: [".js", ".jsx", ".json", ".wasm"],
    fallback: {
      path: require.resolve("path-browserify"),
      fs: false,
    },
  },
};
