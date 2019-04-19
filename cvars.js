const fs = require('fs');

fs.readFile('sar.cvars', 'utf-8', (err, data) => {
    if (err) {
        return console.error(err);
    }

    const body = data
        .replace(/\[end_of_cvar\]|\[cvar_data\]/g, '|')
        .replace(/\n(?!\|)/g, '<br>')
        .slice(0, -1);

    fs.writeFileSync('doc/cvars.md',
`# SAR: Cvars

|Name|Default|Flags|Description|
|---|---|---|---|
|${body}`);
});
