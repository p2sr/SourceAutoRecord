const fs = require('fs');

fs.readFile(process.argv[2] + '/sar.cvars', 'utf-8', (err, data) => {
    if (err) {
        return console.error(err);
    }

    data = '[end_of_cvar]' + data;

    const body = data
        .replace(/\n\[cvar_data\]/g, '<br>[cvar_data\]')
        .replace(/\[end_of_cvar\]|\[cvar_data\]/g, '|')
        .replace(/\n(?!\|)/g, '<br>')
        .slice(0, -1)
        .split('\n')
        .sort((a, b) => {
            a = a.substr(1);
            b = b.substr(1);
            if (a.startsWith('-') || a.startsWith('+')) {
                a = a.substring(1);
            }
            if (b.startsWith('-') || b.startsWith('+')) {
                b = b.substring(1);
            }
            return a.localeCompare(b);
        })
        .join('\n');

    fs.writeFileSync('doc/cvars.md',
`# SAR: Cvars

|Name|Default|Game|Description|
|---|---|---|---|${body}`);
});
