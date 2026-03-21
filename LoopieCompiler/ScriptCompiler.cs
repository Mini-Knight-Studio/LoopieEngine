using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;
using Microsoft.CodeAnalysis.Emit;

namespace Loopie
{
    public static class ScriptCompiler
    {
        public static bool Compile(string sourceDir, string outputDll, string coreDll, out string error)
        {
            error = string.Empty;

            try
            {
                Console.WriteLine("Gathering .cs files for Roslyn...");

                var csFiles = Directory.GetFiles(sourceDir, "*.cs", SearchOption.AllDirectories);
                if (csFiles.Length == 0)
                {
                    error = "No C# files found in source directory.";
                    return false;
                }

                // 1. Parse syntax trees from files
                var syntaxTrees = csFiles.Select(file =>
                    CSharpSyntaxTree.ParseText(File.ReadAllText(file), path: file)).ToArray();

                // 2. Setup references (using absolute paths to avoid Mono search issues)
                var baseLibPath = Path.GetDirectoryName(typeof(object).Assembly.Location);
                var references = new List<MetadataReference>
                {
                    MetadataReference.CreateFromFile(Path.Combine(baseLibPath, "mscorlib.dll")),
                    MetadataReference.CreateFromFile(Path.Combine(baseLibPath, "System.dll")),
                    MetadataReference.CreateFromFile(Path.Combine(baseLibPath, "System.Core.dll"))
                };

                // Add the engine's core DLL if provided
                if (!string.IsNullOrEmpty(coreDll) && File.Exists(coreDll))
                {
                    Console.WriteLine("Adding Loopie.Core...");
                    references.Add(MetadataReference.CreateFromFile(coreDll));
                    references.Add(MetadataReference.CreateFromFile(Path.Combine(Path.GetDirectoryName(coreDll), "Newtonsoft.Json.dll")));
                }

                // 3. Setup compilation options
                var compilation = CSharpCompilation.Create(
                    Path.GetFileNameWithoutExtension(outputDll),
                    syntaxTrees,
                    references,
                    new CSharpCompilationOptions(OutputKind.DynamicallyLinkedLibrary)
                        .WithOptimizationLevel(OptimizationLevel.Debug)
                );

                string outputDir = Path.GetDirectoryName(outputDll);
                if (!string.IsNullOrEmpty(outputDir) && !Directory.Exists(outputDir))
                {
                    Console.WriteLine($"Creating directory: {outputDir}");
                    Directory.CreateDirectory(outputDir);
                }

                // 4. Emit to file
                EmitResult result = compilation.Emit(outputDll);

                if (!result.Success)
                {
                    var failures = result.Diagnostics.Where(diagnostic =>
                        diagnostic.IsWarningAsError ||
                        diagnostic.Severity == DiagnosticSeverity.Error);

                    error = string.Join(Environment.NewLine, failures.Select(d =>
                        $"{d.Location.GetLineSpan().Path}({d.Location.GetLineSpan().StartLinePosition.Line}): {d.GetMessage()}"));

                    return false;
                }

                Console.WriteLine($"DLL compiled successfully via Roslyn: {outputDll}");
                return true;
            }
            catch (Exception ex)
            {
                error = ex.ToString();
                return false;
            }
        }
    }
}