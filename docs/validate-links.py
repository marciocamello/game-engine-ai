#!/usr/bin/env python3
"""
Documentation Link Validation Script for Game Engine Kiro

This script validates internal documentation references to ensure all cross-references
are accurate and up-to-date. It checks for:
- Broken internal links
- Missing referenced documents
- Inconsistent link formatting
- Orphaned documents without incoming references

Usage:
    python docs/validate-links.py
    python docs/validate-links.py --verbose
    python docs/validate-links.py --fix-links (future feature)
"""

import os
import re
import sys
from pathlib import Path
from typing import Dict, List, Set, Tuple

class DocumentationLinkValidator:
    def __init__(self, docs_dir: str = "docs"):
        self.docs_dir = Path(docs_dir)
        self.markdown_files = list(self.docs_dir.glob("*.md"))
        self.link_pattern = re.compile(r'\[([^\]]+)\]\(([^)]+\.md)(?:#[^)]*)?\)')
        self.errors = []
        self.warnings = []
        
    def validate_all_links(self, verbose: bool = False) -> bool:
        """Validate all internal documentation links."""
        print(f"🔍 Validating documentation links in {self.docs_dir}")
        print(f"📄 Found {len(self.markdown_files)} markdown files")
        
        # Build file map
        file_map = {f.name: f for f in self.markdown_files}
        
        # Track all links
        all_links = {}
        incoming_links = {}
        
        for md_file in self.markdown_files:
            if verbose:
                print(f"  📖 Checking {md_file.name}")
                
            links = self._extract_links(md_file)
            all_links[md_file.name] = links
            
            # Check each link
            for link_text, target_file in links:
                if target_file not in file_map:
                    self.errors.append(f"❌ {md_file.name}: Broken link to '{target_file}' (text: '{link_text}')")
                else:
                    # Track incoming links
                    if target_file not in incoming_links:
                        incoming_links[target_file] = []
                    incoming_links[target_file].append((md_file.name, link_text))
        
        # Check for orphaned files (no incoming links except navigation index)
        for md_file in self.markdown_files:
            filename = md_file.name
            if filename == "testing-navigation-index.md":
                continue  # Navigation index is expected to have few incoming links
                
            if filename not in incoming_links:
                self.warnings.append(f"⚠️  {filename}: No incoming links (potential orphan)")
            elif len(incoming_links[filename]) == 1 and incoming_links[filename][0][0] == "testing-navigation-index.md":
                self.warnings.append(f"⚠️  {filename}: Only referenced from navigation index")
        
        # Report results
        self._print_results(verbose)
        
        return len(self.errors) == 0
    
    def _extract_links(self, md_file: Path) -> List[Tuple[str, str]]:
        """Extract all internal markdown links from a file."""
        try:
            content = md_file.read_text(encoding='utf-8')
            matches = self.link_pattern.findall(content)
            return [(text, target) for text, target in matches if target.endswith('.md')]
        except Exception as e:
            self.errors.append(f"❌ Error reading {md_file.name}: {e}")
            return []
    
    def _print_results(self, verbose: bool):
        """Print validation results."""
        print("\n" + "="*60)
        print("📊 VALIDATION RESULTS")
        print("="*60)
        
        if self.errors:
            print(f"\n❌ ERRORS ({len(self.errors)}):")
            for error in self.errors:
                print(f"  {error}")
        
        if self.warnings:
            print(f"\n⚠️  WARNINGS ({len(self.warnings)}):")
            for warning in self.warnings:
                print(f"  {warning}")
        
        if not self.errors and not self.warnings:
            print("\n✅ All documentation links are valid!")
        elif not self.errors:
            print(f"\n✅ No broken links found ({len(self.warnings)} warnings)")
        else:
            print(f"\n❌ Found {len(self.errors)} errors and {len(self.warnings)} warnings")
        
        print("\n" + "="*60)
    
    def generate_link_report(self) -> str:
        """Generate a comprehensive link report."""
        report = []
        report.append("# Documentation Link Report")
        report.append("")
        report.append("## Link Matrix")
        report.append("")
        
        # Build link matrix
        file_map = {f.name: f for f in self.markdown_files}
        link_matrix = {}
        
        for md_file in self.markdown_files:
            links = self._extract_links(md_file)
            link_matrix[md_file.name] = [target for _, target in links]
        
        # Create table
        report.append("| From Document | Links To |")
        report.append("|---------------|----------|")
        
        for source_file in sorted(link_matrix.keys()):
            targets = link_matrix[source_file]
            if targets:
                targets_str = ", ".join(sorted(set(targets)))
                report.append(f"| {source_file} | {targets_str} |")
            else:
                report.append(f"| {source_file} | *(no outgoing links)* |")
        
        return "\n".join(report)

def main():
    """Main entry point."""
    verbose = "--verbose" in sys.argv or "-v" in sys.argv
    
    # Change to project root if running from docs directory
    if os.path.basename(os.getcwd()) == "docs":
        os.chdir("..")
    
    validator = DocumentationLinkValidator()
    
    if "--report" in sys.argv:
        report = validator.generate_link_report()
        with open("docs/link-report.md", "w", encoding='utf-8') as f:
            f.write(report)
        print("📄 Link report generated: docs/link-report.md")
        return
    
    success = validator.validate_all_links(verbose)
    
    if not success:
        print("\n💡 To fix broken links:")
        print("  1. Check that referenced files exist in docs/")
        print("  2. Verify file names match exactly (case-sensitive)")
        print("  3. Update links to use correct file names")
        print("  4. Consider adding missing cross-references")
        sys.exit(1)
    else:
        print("\n🎉 All documentation links validated successfully!")

if __name__ == "__main__":
    main()