#!/usr/bin/env python3

import os
import subprocess
import tempfile
import tkinter as tk
from tkinter import filedialog, messagebox, scrolledtext


class QuantScriptIDE:
    def __init__(self, root):
        self.root = root
        self.root.title("QuantScript IDE")
        self.root.geometry("900x650")
        self.root.minsize(700, 500)

        self.file_path = None

        self.project_root = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "..")
        )
        self.compiler = os.path.join(
            self.project_root, "bin", "quantscript"
        )

        self.create_widgets()
        self.load_default_example()

    def create_widgets(self):
        toolbar = tk.Frame(self.root)
        toolbar.pack(fill=tk.X, padx=8, pady=6)

        buttons = [
            ("New", self.new_file),
            ("Open", self.open_file),
            ("Save", self.save_file),
            ("Run", self.run_program),
            ("Tokens", self.show_tokens),
            ("AST", self.show_ast),
            ("LLVM IR", self.show_ir),
            ("Optimized IR", self.show_optimized_ir),
            ("Clear Output", self.clear_output),
        ]

        for text, command in buttons:
            tk.Button(
                toolbar,
                text=text,
                command=command
            ).pack(side=tk.LEFT, padx=3)

        tk.Label(
            self.root,
            text="QuantScript Source",
            anchor="w"
        ).pack(fill=tk.X, padx=8)

        self.editor = scrolledtext.ScrolledText(
            self.root,
            wrap=tk.NONE,
            undo=True
        )
        self.editor.pack(
            fill=tk.BOTH,
            expand=True,
            padx=8,
            pady=(2, 6)
        )

        tk.Label(
            self.root,
            text="Compiler Output",
            anchor="w"
        ).pack(fill=tk.X, padx=8)

        self.output = scrolledtext.ScrolledText(
            self.root,
            height=12,
            wrap=tk.NONE,
            bg="black",
            fg="white"
        )
        self.output.pack(
            fill=tk.BOTH,
            expand=True,
            padx=8,
            pady=(2, 8)
        )

    def load_default_example(self):
        example = os.path.join(
            self.project_root,
            "examples",
            "mean_reversion.qs"
        )

        if os.path.exists(example):
            self.file_path = example

            try:
                with open(example, "r", encoding="utf-8") as f:
                    self.editor.delete("1.0", tk.END)
                    self.editor.insert("1.0", f.read())

                self.update_title()

            except OSError:
                pass

    def update_title(self):
        if self.file_path:
            name = os.path.basename(self.file_path)
            self.root.title("QuantScript IDE - " + name)
        else:
            self.root.title("QuantScript IDE - Untitled")

    def new_file(self):
        self.editor.delete("1.0", tk.END)
        self.file_path = None
        self.clear_output()
        self.update_title()

    def open_file(self):
        path = filedialog.askopenfilename(
            title="Open QuantScript File",
            filetypes=[
                ("QuantScript files", "*.qs"),
                ("All files", "*.*")
            ]
        )

        if not path:
            return

        try:
            with open(path, "r", encoding="utf-8") as f:
                content = f.read()

            self.editor.delete("1.0", tk.END)
            self.editor.insert("1.0", content)

            self.file_path = path
            self.clear_output()
            self.update_title()

        except OSError as e:
            messagebox.showerror(
                "Open Error",
                str(e)
            )

    def save_file(self):
        if not self.file_path:
            return self.save_file_as()

        return self.write_file(self.file_path)

    def save_file_as(self):
        path = filedialog.asksaveasfilename(
            title="Save QuantScript File",
            defaultextension=".qs",
            filetypes=[
                ("QuantScript files", "*.qs"),
                ("All files", "*.*")
            ]
        )

        if not path:
            return False

        self.file_path = path

        if self.write_file(path):
            self.update_title()
            return True

        return False

    def write_file(self, path):
        try:
            content = self.editor.get("1.0", tk.END)

            with open(path, "w", encoding="utf-8") as f:
                f.write(content)

            self.file_path = path
            self.update_title()
            return True

        except OSError as e:
            messagebox.showerror(
                "Save Error",
                str(e)
            )
            return False

    def ensure_saved(self):
        if self.file_path:
            return self.write_file(self.file_path)

        return self.save_file_as()

    def execute_compiler(self, args):
        if not os.path.exists(self.compiler):
            self.write_output(
                "Error: QuantScript compiler not found:\n"
                + self.compiler
                + "\n"
            )
            return None

        try:
            result = subprocess.run(
                [self.compiler] + args,
                cwd=self.project_root,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT
            )

            self.write_output(result.stdout)

            return result

        except Exception as e:
            self.write_output(
                "Error while running compiler:\n"
                + str(e)
                + "\n"
            )
            return None

    def write_output(self, text):
        self.output.insert(tk.END, text)
        self.output.see(tk.END)
        self.root.update_idletasks()

    def clear_output(self):
        self.output.delete("1.0", tk.END)

    def run_program(self):
        if not self.ensure_saved():
            return

        self.clear_output()

        self.write_output(
            "=== QuantScript Compilation ===\n"
        )

        result = self.execute_compiler(
            [self.file_path]
        )

        if result is None or result.returncode != 0:
            self.write_output(
                "\nCompilation failed.\n"
            )
            return

        self.write_output(
            "\nCompilation successful.\n"
        )

        object_path = None
        runner_path = None
        executable_path = None

        try:
            with tempfile.NamedTemporaryFile(
                suffix=".o",
                delete=False,
                dir=self.project_root
            ) as f:
                object_path = f.name

            with tempfile.NamedTemporaryFile(
                suffix=".c",
                delete=False,
                dir=self.project_root,
                mode="w",
                encoding="utf-8"
            ) as f:
                runner_path = f.name
                f.write(
                    "extern int strategy_main(void);\n"
                    "\n"
                    "int main(void) {\n"
                    "    return strategy_main();\n"
                    "}\n"
                )

            executable_path = os.path.join(
                self.project_root,
                ".quantscript_run"
            )

            self.write_output(
                "\n=== Native Execution ===\n"
            )

            compile_result = subprocess.run(
                [
                    self.compiler,
                    self.file_path,
                    "--emit-object",
                    "-o",
                    object_path
                ],
                cwd=self.project_root,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT
            )

            if compile_result.returncode != 0:
                self.write_output(compile_result.stdout)
                self.write_output(
                    "\nObject generation failed.\n"
                )
                return

            link_result = subprocess.run(
                [
                    "gcc",
                    "-no-pie",
                    runner_path,
                    object_path,
                    "-o",
                    executable_path
                ],
                cwd=self.project_root,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT
            )

            if link_result.returncode != 0:
                self.write_output(link_result.stdout)
                self.write_output(
                    "\nNative linking failed.\n"
                )
                return

            run_result = subprocess.run(
                [executable_path],
                cwd=self.project_root,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT
            )

            self.write_output(
                "Program output:\n"
            )

            self.write_output(
                run_result.stdout
            )

            if run_result.returncode == 0:
                self.write_output(
                    "\nExecution successful.\n"
                )
            else:
                self.write_output(
                    "\nProgram exited with code "
                    + str(run_result.returncode)
                    + ".\n"
                )

        except Exception as e:
            self.write_output(
                "\nExecution error:\n"
                + str(e)
                + "\n"
            )

        finally:
            for path in [
                object_path,
                runner_path,
                executable_path
            ]:
                if path and os.path.exists(path):
                    try:
                        os.remove(path)
                    except OSError:
                        pass

    def show_tokens(self):
        if not self.ensure_saved():
            return

        self.clear_output()
        self.execute_compiler(
            [self.file_path, "--tokens"]
        )

    def show_ast(self):
        if not self.ensure_saved():
            return

        self.clear_output()
        self.execute_compiler(
            [self.file_path, "--ast"]
        )

    def show_ir(self):
        if not self.ensure_saved():
            return

        self.clear_output()
        self.execute_compiler(
            [self.file_path, "--ir"]
        )

    def show_optimized_ir(self):
        if not self.ensure_saved():
            return

        self.clear_output()
        self.execute_compiler(
            [self.file_path, "--optimized-ir"]
        )


def main():
    root = tk.Tk()
    QuantScriptIDE(root)
    root.mainloop()


if __name__ == "__main__":
    main()
