# Uso: Ler cracha e imprimir informacoes

O firmware faz duas coisas:
1. Le o UID do cracha/cartao quando aproximado do RC522
2. Procura esse UID em uma lista (cadastro) e imprime as informacoes

## Passo a passo

1. Suba o firmware e abra o monitor serial:

```bash
pio run -t upload
pio device monitor -b 9600
```

2. Aproxime o cracha e copie o UID exibido, por exemplo:

`UID: A1:B2:C3:D4`

3. Cadastre o UID no codigo

Arquivo: `src/main.cpp`

Procure por `static const Entry entries[]` e adicione uma linha no formato:

```cpp
{"A1:B2:C3:D4", "Fulano da Silva", "Funcionario"},
```

4. Recompile e envie novamente

5. Teste de novo aproximando o cracha

Se o UID estiver cadastrado, vai imprimir `UID`, `Nome` e `Cargo`.

## Dicas

- O UID e impresso em HEX com `:` e em maiusculo. Cadastre exatamente no mesmo formato.
- Alguns cartoes tem UID com 7 ou 10 bytes (vai aparecer maior).
