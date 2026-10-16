// ==========================================
// RACK MONITOR - TESTES
// ==========================================
const request = require("supertest");
const app = require("./app");

// Silencia logs durante os testes
beforeAll(() => {
  jest.spyOn(console, "log").mockImplementation(() => {});
  jest.spyOn(console, "error").mockImplementation(() => {});
});

afterAll(() => {
  console.log.mockRestore();
  console.error.mockRestore();
});

beforeEach(() => {
  // Reset do estado entre testes (evita interferência)
  app.__resetState();
});

// ─────────────── /api/health ───────────────
describe("GET /api/health", () => {
  it("retorna 200 com status ok", async () => {
    const r = await request(app).get("/api/health");
    expect(r.status).toBe(200);
    expect(r.body.status).toBe("ok");
    expect(typeof r.body.uptime).toBe("number");
  });
});

// ─────────────── /api/status (POST) ───────────────
describe("POST /api/status", () => {
  const payloadValido = {
    timestamp: "16/10 11:36:16",
    modo: "ARMADO",
    estado: "SEGURO",
    porta: "fechada",
    armado: true,
    acessos: 0,
    bloqueios: 0,
    invasoes: 0,
    uid: "-",
  };

  it("aceita payload valido e retorna { ok: true }", async () => {
    const r = await request(app).post("/api/status").send(payloadValido);
    expect(r.status).toBe(200);
    expect(r.body).toEqual({ ok: true });
  });

  it("atualiza estadoAtual apos POST", async () => {
    await request(app).post("/api/status").send(payloadValido);
    const r = await request(app).get("/api/status");
    expect(r.body.modo).toBe("ARMADO");
    expect(r.body.estado).toBe("SEGURO");
    expect(r.body.porta).toBe("fechada");
    expect(r.body.ultimaAtualizacao).not.toBeNull();
  });

  it("registra evento MUDANCA_ESTADO quando estado muda", async () => {
    // Estado inicial: DESCONHECIDO/DESCONHECIDO
    // Primeiro POST com SEGURO -> deve gerar 1 evento
    await request(app).post("/api/status").send(payloadValido);
    const r = await request(app).get("/api/status");
    const mudancas = r.body.eventos.filter((e) => e.tipo === "MUDANCA_ESTADO");
    expect(mudancas.length).toBe(1);
    expect(mudancas[0].para).toBe("ARMADO/SEGURO");
  });

  it("NAO registra evento quando estado permanece igual", async () => {
    await request(app).post("/api/status").send(payloadValido);
    await request(app).post("/api/status").send(payloadValido); // mesmo estado
    const r = await request(app).get("/api/status");
    const mudancas = r.body.eventos.filter((e) => e.tipo === "MUDANCA_ESTADO");
    expect(mudancas.length).toBe(1); // só a primeira mudança
  });

  it("limita historico a 20 itens no GET", async () => {
    for (let i = 0; i < 25; i++) {
      await request(app).post("/api/status").send(payloadValido);
    }
    const r = await request(app).get("/api/status");
    expect(r.body.historico.length).toBe(20);
  });
});

// ─────────────── /api/comando ───────────────
describe("POST /api/comando", () => {
  it("aceita comando valido e retorna { ok: true }", async () => {
    const r = await request(app)
      .post("/api/comando")
      .send({ comando: "ARMAR" });
    expect(r.status).toBe(200);
    expect(r.body).toEqual({ ok: true });
  });

  it("retorna 400 quando comando ausente", async () => {
    const r = await request(app).post("/api/comando").send({});
    expect(r.status).toBe(400);
    expect(r.body.erro).toBe("comando obrigatorio");
  });

  it("retorna 400 quando body vazio", async () => {
    const r = await request(app)
      .post("/api/comando")
      .set("Content-Type", "application/json")
      .send("");
    expect(r.status).toBe(400);
  });

  it("registra evento COMANDO_WEB", async () => {
    await request(app).post("/api/comando").send({ comando: "DESARMAR" });
    const r = await request(app).get("/api/status");
    const cmdEventos = r.body.eventos.filter((e) => e.tipo === "COMANDO_WEB");
    expect(cmdEventos.length).toBe(1);
    expect(cmdEventos[0].comando).toBe("DESARMAR");
  });
});

describe("GET /api/comando", () => {
  it("retorna { comando: null } quando fila vazia", async () => {
    const r = await request(app).get("/api/comando");
    expect(r.status).toBe(200);
    expect(r.body).toEqual({ comando: null });
  });

  it("retorna o comando enfileirado", async () => {
    await request(app).post("/api/comando").send({ comando: "ARMAR" });
    const r = await request(app).get("/api/comando");
    expect(r.body.comando).toBe("ARMAR");
  });

  it("consome o comando (proximo GET retorna null)", async () => {
    await request(app).post("/api/comando").send({ comando: "ARMAR" });
    await request(app).get("/api/comando"); // consome
    const r = await request(app).get("/api/comando");
    expect(r.body.comando).toBeNull();
  });
});

// ─────────────── /api/stats ───────────────
describe("GET /api/stats", () => {
  it("retorna contadores", async () => {
    await request(app).post("/api/status").send({
      modo: "ARMADO",
      estado: "SEGURO",
      porta: "fechada",
    });
    const r = await request(app).get("/api/stats");
    expect(r.status).toBe(200);
    expect(r.body.totalStatus).toBe(1);
    expect(typeof r.body.uptime).toBe("number");
  });
});

// ─────────────── Fluxo end-to-end ───────────────
describe("Fluxo completo ESP32 + Navegador", () => {
  it("ESP32 envia status, navegador envia comando, ESP32 consome", async () => {
    // 1. ESP32 envia status
    await request(app).post("/api/status").send({
      timestamp: "16/10 12:00:00",
      modo: "ARMADO",
      estado: "SEGURO",
      porta: "fechada",
    });

    // 2. Navegador consulta e vê o estado
    let r = await request(app).get("/api/status");
    expect(r.body.modo).toBe("ARMADO");

    // 3. Navegador envia comando
    await request(app).post("/api/comando").send({ comando: "DESARMAR" });

    // 4. ESP32 consulta e recebe o comando
    r = await request(app).get("/api/comando");
    expect(r.body.comando).toBe("DESARMAR");

    // 5. ESP32 envia novo status (DESARMADO)
    await request(app).post("/api/status").send({
      timestamp: "16/10 12:00:20",
      modo: "DESARMADO",
      estado: "SEGURO",
      porta: "fechada",
    });

    // 6. Navegador vê o estado atualizado + evento de mudança
    r = await request(app).get("/api/status");
    expect(r.body.modo).toBe("DESARMADO");
    const mudancas = r.body.eventos.filter((e) => e.tipo === "MUDANCA_ESTADO");
    expect(mudancas.length).toBe(2); // ARMADO/SEGURO + DESARMADO/SEGURO
  });
});
