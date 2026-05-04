# Terms of Use for `mnu`

Welcome to `mnu`! We built this tool to make reading terminal documentation simple, fast, and helpful. To ensure the tool remains a reliable resource for everyone and respects the infrastructure it relies upon, we require all users to agree to these Terms of Use.

By downloading, compiling, or using the `mnu` client, you agree to comply with the following formal terms.

## 1. Acceptable Use & Architecture
`mnu` is designed to fetch and display structured text files (the `.mn` format). We built `mnu` to fetch pages instantly and directly from GitHub. We chose this direct-hosting approach because downloading entire documentation repositories locally is often impractical, consumes unnecessary disk space, and quickly becomes outdated. 

Because we rely on this live-fetching architecture, you are encouraged to use the client for learning, debugging, and reading documentation in a standard, responsible manner that respects network resources.

## 2. Prohibited Activities
To protect the integrity of the project and our hosting provider (GitHub), certain actions are strictly prohibited. You agree that you will not use `mnu` to engage in abuse, **including but not limited to**:

*   **Spamming and Rate Limiting:** Using scripts, bots, or automated processes to rapidly fetch pages, thereby spamming GitHub's infrastructure or intentionally triggering rate limits.
*   **Malicious Payloads:** Attempting to use the client to fetch, parse, or execute malicious code or exploit server-side vulnerabilities.
*   **Denial of Service:** Intentionally attempting to crash the client, disrupt the network, or perform Denial of Service (DoS) attacks against the source repositories.
*   **Misrepresentation:** Altering the client's internal headers (such as the User-Agent) to bypass security controls while claiming to be the official `mnu` client.

## 3. Anonymous Identifiers
To protect our hosting provider (GitHub) from abuse and to help us understand general usage patterns without compromising your privacy, the `mnu` client generates a **Privacy-Preserving Anonymous ID**.

*   **Format:** The ID is a unique **8-character hexadecimal string** (e.g., `5a2f8c9d`).
*   **Zero Footprint:** This ID is generated in memory every time the application runs. It is **never** saved to your disk.
*   **Privacy-First:** The ID is a one-way mathematical hash of non-private system information (such as your OS type and machine architecture). It contains no personal data, usernames, or IP addresses.
*   **Purpose:** This ID is sent in the `User-Agent` header solely to help distinguish between unique clients for rate-limiting and security purposes.

## 4. Content Guidelines
The pages displayed by `mnu` are community-driven and written to strict formatting terms to ensure consistency and simplicity. Because the content is sourced from the community, we rely on users to help maintain its quality. 

If you encounter any prohibited content, harmful instructions, or incorrect information, please always feel free to report it on our repository so it can be promptly addressed by the maintainers.

## 5. Limitation of Liability
In accordance with our MIT License, the software is provided "as is", without warranty of any kind. Under no circumstances shall the authors, maintainers, or copyright holders be liable for any claim, damages, or other liability arising from your use of the software, or from any actions you take based on the documentation it displays.

## 6. Changes to Terms
We reserve the right to update these terms at any time. Continued use of `mnu` following any changes indicates your formal acceptance of the new terms.

---
*Thank you for using `mnu` responsibly and helping us keep the terminal simple!*
