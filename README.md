# SudokuSolver

Este projeto implementa um resolvedor de Sudoku em C++23 com duas abordagens principais: **Backtracking** e **Dancing Links (Algorithm X)**. Há ainda uma pequena interface utilizando **SFML** para visualizar o tabuleiro, bem como testes e benchmarks.

## Dependências

As dependências são gerenciadas via [vcpkg](https://github.com/microsoft/vcpkg) (o repositório já inclui o submódulo). As principais bibliotecas utilizadas são:

- SFML
- Boost (dynamic_bitset)
- pcg
- GoogleTest
- Google Benchmark

Certifique-se de inicializar o submódulo após clonar o repositório:

```bash
git submodule update --init --recursive
```

Em ambientes Linux pode ser necessário instalar alguns utilitários para que o
`vcpkg` consiga compilar todas as bibliotecas, em especial o port `alsa`.
Execute:

```bash
sudo apt-get install autoconf automake libtool
```

## Como compilar

Utilize o **CMake** (versão >= 3.30.0) e um compilador com suporte a C++23.

```bash
# Configuração
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE=Release

# Compilação
cmake --build build -j$(nproc)
```

No Windows é possível usar os `presets` já configurados:

```bash
cmake --preset VisualStudio-release
cmake --build out/build/VisualStudio-release --config Release
```

## Executando

Após a compilação o executável principal estará em `build/SudokuSolver` (ou na pasta correspondente do preset). Para executar:

```bash
./build/SudokuSolver
```

Também é possível rodar os benchmarks:

```bash
./build/SudokuSolver_BENCHMARK
```

## Testes

Os testes unitários usam GoogleTest. Para gerar e executar:

```bash
cmake --build build --target SudokuSolver_TEST
ctest --test-dir build --output-on-failure
```

## Licença

Este projeto está licenciado sob os termos da licença MIT. Veja o arquivo [LICENSE](LICENSE) para mais detalhes.

## Contribuindo

Pull requests são bem-vindos! Sinta-se livre para abrir *issues* ou propor melhorias.
