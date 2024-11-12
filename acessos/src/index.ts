import * as readline from "readline";

function acessoSequencial(array: Int32Array): number {
  let total = 0;
  for (let i = 0; i < array.length; i++) {
    total += array[i];
  }

  return total;
}


function acessoAleatorio(array: Int32Array): number {
  const indices = Array.from({ length: array.length }, (_, i) => i);
  for (let i = indices.length - 1; i > 0; i--) {
    const j = Math.floor(Math.random() * (i + 1));
    [indices[i], indices[j]] = [indices[j], indices[i]];
  }

  let total = 0;
  for (const i of indices) {
    total += array[i];
  }
  return total;
}

function main(): void {
  const tamanho = 100_000_000;
  const array = new Int32Array(tamanho).fill(1);
  console.log(`PID do processo: ${process.pid}`);

  console.log("\nIniciando acesso sequencial...");
  const inicioSeq = Date.now();
  const totalSeq = acessoSequencial(array);
  const tempoSeq = (Date.now() - inicioSeq) / 1000;
  console.log(
    `\n\tTempo de acesso sequencial: ${tempoSeq.toFixed(2)} segundos`
  );

  console.log("\nIniciando acesso aleatório...");
  const inicioAleatorio = Date.now();
  const totalAleatorio = acessoAleatorio(array);
  const tempoAleatorio = (Date.now() - inicioAleatorio) / 1000;
  console.log(
    `\n\tTempo de acesso aleatório: ${tempoAleatorio.toFixed(2)} segundos`
  );

  console.log(
    `\n\tTotal sequencial: ${totalSeq}, Total aleatório: ${totalAleatorio}`
  );

  const rl = readline.createInterface({
    input: process.stdin,
    output: process.stdout,
  });

  rl.question("\nPressione Enter para sair...", () => {
    rl.close();
  });
}

main();
