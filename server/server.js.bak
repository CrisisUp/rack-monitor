// ==========================================
// RACK MONITOR - SERVER (entry point)
// ==========================================
const app = require("./app");
const path = require("path");

const PORT = process.env.PORT || 3000;
const DATA_DIR = path.join(__dirname, "data");

app.listen(PORT, "0.0.0.0", () => {
  console.log("=================================================");
  console.log("  RACK MONITOR SERVER");
  console.log("=================================================");
  console.log(`  Porta:      ${PORT}`);
  console.log(`  URL local:  http://localhost:${PORT}`);
  console.log(`  Dados em:   ${DATA_DIR}`);
  console.log("=================================================");
});
